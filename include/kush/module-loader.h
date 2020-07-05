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

#ifndef KUSH_MODULE_LOADER_H
#define KUSH_MODULE_LOADER_H

/*******************************************************************************
 * ModuleLoader                                                                *
 *******************************************************************************/

#include <kush/configuration.h>

#include <kush/scope.h>
#include <jtk/collection/map/HashMap.h>

/*******************************************************************************
 * ModuleLoader                                                                *
 *******************************************************************************/

typedef struct Compiler Compiler;

struct ModuleLoader {

    /**
     * The list of directories where the loader looks for the definitions
     * of entities.
     */
    uint8_t* directories;

    int32_t directoryCount;

    /**
     * Cache to store entities loaded previously.
     */
    jtk_HashMap_t* symbols;

    Compiler* compiler;
};

typedef struct ModuleLoader ModuleLoader;

// Constructor

ModuleLoader* newModuleLoader(Compiler* compiler, uint8_t** directories,
    int32_t directoryCount);

// Destructor

void deleteModuleLoader(ModuleLoader* loader);

// Find

/**
 * First, it tries to find a previously loaded class with the specified
 * descriptor in the class registry. If not found, it tries to load it
 * from a physical description, i.e., a binary entity. It fails if a
 * corresponding binary entity is not found, without raising any exception.
 */
Symbol* loadSymbol(ModuleLoader* loader, const uint8_t* descriptor,
    int32_t descriptorSize);


#endif /* KUSH_MODULE_LOADER_H */
