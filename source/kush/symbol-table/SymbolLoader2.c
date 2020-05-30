// Sunday, May 30, 2020

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <kush/AbstractModule.h>


int SymbolLoader()
{
	FILE *f = fopen("object", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  

	uint8_t *bytes = malloc(fsize + 1);
	
	/*
	 *  READ BINARY FORMATS WITHOUT USING fread()
	 */
	  
	fclose(f);

	bytes[fsize] = 0;
	
	int i = 0;
	uint16_t magicNumber = bytes[i++] << 24 | bytes[i++] << 16 | bytes[i++] << 8 | bytes[i++];
	uint16_t majorVersion = bytes[i++] << 8 | bytes[i++];
	uint16_t minorVersion = bytes[i++] << 8 | bytes[i++];
	
	uint16_t structureCount = bytes[i++] << 8 | bytes[i++];	
	k_Structure_t** structures;
	for(int i = 0; i < structureCount; ++i) {
		structures[i] = parseStructure();
	}
	
	uint16_t functionCount = bytes[i++] << 8 | bytes[i++];
	k_Function_t** functions;
	for(int i = 0; i < functionCount; ++i) {
		functions[i] = parseFunction();
	}
	
	k_Module_t* k = malloc(sizeof(k_Module_t));
	k->m_magicNumber = magicNumber;
	k->m_majorVersion = majorVersion;
	k->m_minorVersion = minorVersion;
	k->m_structureCount = structureCount;
	k->m_functionCount = functionCount;
	
	// Serializing Structures
	for(int i = 0; i < k->m_structureCount; ++i) {
		k->m_structures = structures[i];
	}
	
	// Serializing Functions
	for(int i = 0; i < k->m_functionCount; ++i) {
		k->m_functions = functions[i];
	}
	
	k_Structure_t* parseStructure() {
		uint16_t flags = bytes[i++] << 8 | bytes[i++];
		
		uint16_t nameSize = bytes[i++] << 8 | bytes[i++];
		uint8_t* name = malloc(nameSize*sizeof(uint8_t);
		for(int j = 0; j < nameSize; ++j) {
			name[j] = bytes[i++];
		}
		
		uint16_t attributeCount = bytes[i++] << 8 | bytes[i++];
		uint16_t* attributeNameSizes = malloc(attributeCount*sizeof(uint16_t));
		for(int j = 0; j < attributeCount; ++j) {
			attributeNameSizes[j] = malloc(sizeof(uint16_t));
			attributeNameSizes[j] = bytes[i++] << 8 | bytes[i++];
		}
		
		uint8_t** attributeNames = malloc(attributeCount*sizeof(uint8_t*));
		for(int j = 0; j < attributeCount; ++j) {
			attributeNames[j] = malloc(attributeNameSizes[j]*sizeof(uint8_t));
			for(int k = 0; k < attributeNameSizes[j]; ++k) {
				attributeNames[j][k] = bytes[i++];			
			}
		}
	
		k_Structure_t* k = malloc(sizeof(k_Structure_t));
		k->m_flags = flags;
		k->m_nameSize = nameSize;
		k->m_name = name;
		k->m_attributeCount = attributeCount;
		k->m_attributeNameSizes = attributeNameSizes;
		k->m_attributeNames = attributeNames;
		return k; 
	}
	
	k_Function_t* parseFunction() {
		uint16_t flags = (bytes[i++] << 8) | (bytes[i++]);
		
		uint16_t nameSize = (bytes[i++] << 8) | (bytes[i++]);
		uint8_t* name = malloc(nameSize*sizeof(uint8_t);
		for(int j = 0; j < nameSize; ++j) {
			name[j] = bytes[i++];
		}
		
		uint16_t signatureSize = bytes[i++] << 8 | bytes[i++];
		uint8_t* signature = malloc(signatureSize*sizeof(uint8_t));
		for(int j = 0; j < signatureSize; ++j) {
			signature[j] = bytes[i++];
		}
		
		uint8_t captureCount = bytes[i++];
		uint8_t* captures = malloc(captureCount*sizeof(uint8_t));
		for(int j = 0; j < captureCount; ++j) {
			captures[j] = bytes[i++];
		}
		
		k_Function_t* k = malloc(sizeof(k_Function_t));
		k->m_flags = flags;
		k->m_nameSize = nameSize;
		k->m_name = name;
		k->m_signatureSize = signatureSize;
		k->m_signature = signature;
		k->m_captureCount = captureCount;
		k->m_captures = captures;
		return k;
	}
	return k;
}
