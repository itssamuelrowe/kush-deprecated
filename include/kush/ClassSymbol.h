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

// Sunday, January 07, 2018

#ifndef KUSH_COMPILER_SYMBOL_TABLE_CLASS_SYMBOL_H
#define KUSH_COMPILER_SYMBOL_TABLE_CLASS_SYMBOL_H

#include <jtk/collection/list/ArrayList.h>

/*******************************************************************************
 * ClassSymbol                                                                 *
 *******************************************************************************/

typedef struct k_Scope_t k_Scope_t;

/**
 * @class ClassSymbol
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassSymbol_t {
    /* The superclasses are added in the resolution phase. */
    jtk_ArrayList_t* superClasses; /* <k_Symbol_t*> */
    uint8_t* qualifiedName;
    int32_t qualifiedNameSize;
    k_Scope_t* classScope;

    uint8_t* descriptor;
    int32_t descriptorSize;
};

/**
 * @memberof ClassSymbol
 */
typedef struct k_ClassSymbol_t k_ClassSymbol_t;

void k_ClassSymbol_initialize(k_ClassSymbol_t* symbol);

void k_ClassSymbol_destroy(k_ClassSymbol_t* symbol);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_CLASS_SYMBOL_H */
