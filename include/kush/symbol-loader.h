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

// Saturday, April 06, 2019

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_LOADER_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_LOADER_H

/*******************************************************************************
 * SymbolLoader                                                                *
 *******************************************************************************/

#include <kush/Configuration.h>
#include <kush/symbol-table/Symbol.h>

#include <jtk/collection/Iterator.h>
#include <jtk/collection/map/HashMap.h>
#include <jtk/collection/list/DoublyLinkedList.h>
#include <jtk/fs/Path.h>

#define KUSH_ENTITY_LOADER_FLAG_PRIORITIZE_DIRECTORIES (1 << 0)
#define KUSH_ENTITY_LOADER_FLAG_IGNORE_CORRUPT_ENTITY (1 << 1)

/*******************************************************************************
 * SymbolLoader                                                                *
 *******************************************************************************/

#define KUSH_ENTITY_LOADER_DEFAULT_ENTITIES_MAP_CAPCITY 128

/* A small experiment on a deployed project written in a certain virtual
 * machine powered language revealed that on an average most binary files
 * are 3 kilobytes in size. The buffer size was evaluated to reduce the cache
 * misses and increase the cache hits in the buffered input stream.
 */
#define KUSH_ENTITY_LOADER_BUFFER_SIZE (3 * 1024)

typedef struct k_Compiler_t k_Compiler_t;

/**
 * @class SymbolLoader
 * @ingroup k_virtual_machine_loader
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_SymbolLoader_t {

    /**
     * The list of directories where the loader looks for the definitions
     * of entities.
     */
    jtk_DoublyLinkedList_t* directories;

    uint32_t flags;

    /**
     * Cache to store entities loaded previously.
     */
    jtk_HashMap_t* symbols;

    k_Compiler_t* compiler;

    int32_t index;
    uint8_t* bytes;
    int32_t size;
    k_Symbol_t* symbol;
};

/**
 * @memberof SymbolLoader
 */
typedef struct k_SymbolLoader_t k_SymbolLoader_t;

/* Constructor */

k_SymbolLoader_t* k_SymbolLoader_new(k_Compiler_t* compiler);
k_SymbolLoader_t* k_SymbolLoader_newWithEntityDirectories(k_Compiler_t* compiler,
    jtk_Iterator_t* entityDirectoryIterator);

/* Destructor */

void k_SymbolLoader_delete(k_SymbolLoader_t* loader);

/* Directory */

bool k_SymbolLoader_addDirectory(k_SymbolLoader_t* loader, const uint8_t* directory);

// Find Symbol

/**
 * First, it tries to find a previously loaded class with the specified
 * descriptor in the class registry. If not found, it tries to load it
 * from a physical description, i.e., a binary entity. It fails if a
 * corresponding binary entity is not found, without raising any exception.
 *
 * @memberof SymbolLoader
 */

k_Symbol_t* k_SymbolLoader_findSymbol(k_SymbolLoader_t* loader,
    const uint8_t* descriptor, int32_t descriptorSize);

/**
 * It tries to load a class with the specified descriptor from a physical
 * description, i.e., a binary entity. It fails if a class was previously
 * loaded, or if a corresponding binary entity is not found.
 *
 * @memberof SymbolLoader
 */
k_Symbol_t* k_SymbolLoader_loadSymbol(k_SymbolLoader_t* loader,
    const uint8_t* descriptor, int32_t descriptorSize);

/**
 * It tries to load a class from the specified regular file path. If the file
 * does not exist or is corrupt, it fails without raising an exception.
 *
 * @memberof SymbolLoader
 */
k_Symbol_t* k_SymbolLoader_loadEntityFromHandle(k_SymbolLoader_t* loader,
    jtk_PathHandle_t* handle);

/**
 * It tries to load a class from the specified regular file handle. If the file
 * does not exist or is corrupt, it fails without raising an exception.
 *
 * @memberof SymbolLoader
 */
k_Symbol_t* k_SymbolLoader_loadSymbolFromHandle(k_SymbolLoader_t* loader,
    jtk_PathHandle_t* handle);

k_Symbol_t* k_SymbolLoader_parse(k_SymbolLoader_t* symbolLoader, uint8_t* bytes,
    int32_t size);

// Ignore Corrupt Entity

bool k_SymbolLoader_shouldIgnoreCorruptEntity(k_SymbolLoader_t* loader);
void k_SymbolLoader_setIgnoreCorruptEntity(k_SymbolLoader_t* loader, bool ignoreCorruptEntity);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_LOADER_H */
