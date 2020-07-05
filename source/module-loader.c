/*
 * Copyright 2017-2020 Samuel Rowe, Joel E. Rego
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Saturday, April 10, 2020

#include <jtk/collection/Iterator.h>
#include <jtk/collection/array/Arrays.h>
#include <jtk/collection/array/ByteArray.h>
#include <jtk/collection/list/DoublyLinkedList.h>
#include <jtk/fs/Path.h>
#include <jtk/fs/PathHandle.h>
#include <jtk/fs/FileInputStream.h>
#include <jtk/io/BufferedInputStream.h>
#include <jtk/io/InputStream.h>
#include <jtk/io/InputStreamHelper.h>
#include <jtk/core/CString.h>
#include <jtk/core/CStringObjectAdapter.h>

#include <kush/module-loader.h>
#include <kush/context.h>
#include <kush/compiler.h>

#define LOADER_CAPACITY 128

ModuleLoader* newModuleLoader(Compiler* compiler, uint8_t** directories,
    int32_t directoryCount) {
    jtk_ObjectAdapter_t* adapter = jtk_CStringObjectAdapter_getInstance();

    ModuleLoader* loader = allocate(ModuleLoader, 1);
    loader->directories = directories;
    loader->symbols = jtk_HashMap_newEx(adapter, NULL,
        LOADER_CAPACITY, JTK_HASH_MAP_DEFAULT_LOAD_FACTOR);
    loader->compiler = compiler;

    return loader;
}

void deleteModuleLoader(ModuleLoader* loader) {
    jtk_Assert_assertObject(loader, "The specified entity loader is null.");

    jtk_Iterator_t* entryIterator = jtk_HashMap_getEntryIterator(loader->symbols);
    while (jtk_Iterator_hasNext(entryIterator)) {
        jtk_HashMapEntry_t* entry = (jtk_HashMapEntry_t*)jtk_Iterator_getNext(entryIterator);
        jtk_CString_delete((uint8_t*)entry->m_key);
        deleteSymbol(entry->m_value);
    }
    jtk_Iterator_delete(entryIterator);
    jtk_HashMap_delete(loader->symbols);

    deallocate(loader);
}

// Load Symbol

Symbol* loadSymbol(ModuleLoader* loader, const uint8_t* descriptor,
    int32_t descriptorSize) {
    jtk_Assert_assertObject(loader, "The specified entity loader is null.");
    jtk_Assert_assertObject(descriptor, "The specified descriptor is null.");

    Symbol* result = (Symbol*)jtk_HashMap_getValue(loader->symbols, descriptor);
    if (result == NULL) {
        result = loadFromFile(loader, descriptor, descriptorSize);
    }

    return result;
}

Symbol* loadFromFile(ModuleLoader* loader, const uint8_t* descriptor,
    int32_t descriptorSize) {
    Symbol* result = NULL;
    int32_t fileNameSize;
    uint8_t* fileName = jtk_CString_join(descriptor, ".am", &fileNameSize);
    jtk_Arrays_replaceEx_b((int8_t*)fileName, fileNameSize, '.', '/', 0, fileNameSize - 3);

    int32_t i;
    for (i = 0; i < loader->directoryCount; i++) {
        uint8_t* directory = loader->directories[i];
        int32_t pathSize;
        uint8_t* path = jtk_CString_join(directory, fileName, pathSize);
        FILE* input = fopen(path, "rb");
        if (input != NULL) {
            int32_t size;
            uint8_t* bytes = loadFile(&size);
            result = readModule(loader, bytes, size);
            if (result != NULL) {
                uint8_t* identifier = jtk_CString_newEx(descriptor, descriptorSize);
                jtk_HashMap_put(loader->symbols, identifier, result);
            }
            else {
                /* At this point, the entity loader found an entity file. Unfortunately, the
                * entity file is corrupted. The entity loader will continue to look for entities
                * in different files
                */
                printf("[warn] Corrupt module file '%s'\n", path);
            }
        }
    }
    jtk_CString_delete(fileName);

    return result;
}

uint8_t* loadFile(FILE* file, int32_t* size) {
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t* bytes = allocate(uint8_t, *size);
    fread(bytes, sizeof (uint8_t), *size, file);
    return bytes;
}

void readModule(ModuleLoader* loader, uint8_t* bytes, int32_t size) {
	int i = 0;
	uint16_t magicNumber = (bytes[i++] << 24) | (bytes[i++] << 16) | (bytes[i++] << 8) | bytes[i++];
	uint16_t majorVersion = (bytes[i++] << 8) | bytes[i++];
	uint16_t minorVersion = (bytes[i++] << 8) | bytes[i++];

	uint16_t structureCount = bytes[i++] << 8 | bytes[i++];
	Structure** structures = allocate(Structure*, structureCount);
	for(int i = 0; i < structureCount; ++i) {
        uint16_t flags = bytes[i++] << 8 | bytes[i++];

		uint16_t nameSize = bytes[i++] << 8 | bytes[i++];
		uint8_t* name = allocate(uint8_t, nameSize + 1);
        int32_t j;
		for(j = 0; j < nameSize; j++) {
			name[j] = bytes[i++];
		}
        name[j] = '\0';

		uint16_t attributeCount = bytes[i++] << 8 | bytes[i++];
		uint16_t* attributeNameSizes = allocate(uint16_t, attributeCount);
		for (j = 0; j < attributeCount; j++) {
			attributeNameSizes[j] = (bytes[i++]) << 8 | bytes[i++];
		}

		uint8_t** attributeNames = malloc(attributeCount*sizeof(uint8_t*));
		for (int j = 0; j < attributeCount; j++) {
			attributeNames[j] = allocate(uint8_t, attributeNameSizes[j]);
            int32_t k;
			for(k = 0; k < attributeNameSizes[j]; k++) {
				attributeNames[j][k] = bytes[i++];
			}
            attributeNames[j][k] = '\0';
		}

		Structure* structure = allocate(Structure, 1);
		structure->flags = flags;
		structure->nameSize = nameSize;
		structure->name = name;
		structure->attributeCount = attributeCount;
		structure->attributeNameSizes = attributeNameSizes;
		structure->attributeNames = attributeNames;

        structures[i] = structure;
	}

	uint16_t functionCount = (bytes[i++] << 8) | bytes[i++];
	Function** functions = allocate(Function*, functionCount);
	for(int i = 0; i < functionCount; ++i) {
		uint16_t flags = (bytes[i++] << 8) | (bytes[i++]);

		uint16_t nameSize = (bytes[i++] << 8) | (bytes[i++]);
		uint8_t* name = allocate(uint8_t, nameSize);
        int32_t j;
		for(j = 0; j < nameSize; j++) {
			name[j] = bytes[i++];
		}
        name[j] = '\0';

		uint16_t signatureSize = bytes[i++] << 8 | bytes[i++];
		uint8_t* signature = allocate(uint8_t, signatureSize);
        int32_t j;
		for(j = 0; j < signatureSize; j++) {
			signature[j] = bytes[i++];
		}
        signature[j] = '\0';

		uint8_t captureCount = bytes[i++];
		uint8_t* captures = allocate(uint8_t, captureCount);
		for(int j = 0; j < captureCount; j++) {
			captures[j] = bytes[i++];
		}

		Function* function = allocate(Function, 1);
		function->flags = flags;
		function->nameSize = nameSize;
		function->name = name;
		function->signatureSize = signatureSize;
		function->signature = signature;
		function->captureCount = captureCount;
		function->captures = captures;

        functions[i] = function;
	}

	Module* module = allocate(Module, 1);
	module->magicNumber = magicNumber;
	module->majorVersion = majorVersion;
	module->minorVersion = minorVersion;
	module->structureCount = structureCount;
    module->structures = structures;
	module->functionCount = functionCount;
    module->functions = functions;

	return module;
}