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

#ifndef KUSH_COMPILER_SYMBOL_TABLE_MEMBER_FUNCTION_SYMBOL_H
#define KUSH_COMPILER_SYMBOL_TABLE_MEMBER_FUNCTION_SYMBOL_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/symbol-table/FunctionSignature.h>

/*******************************************************************************
 * FunctionSymbol                                                              *
 *******************************************************************************/

/**
 * @class FunctionSymbol
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionSymbol_t {
    /* Signatures are destroyed by the destructor. */
    jtk_ArrayList_t* m_signatures; /* <k_FunctionSignature_t*> */
    int32_t m_parameterThreshold;
};

/**
 * @memberof FunctionSymbol
 */
typedef struct k_FunctionSymbol_t k_FunctionSymbol_t;

// Initializer

void k_FunctionSymbol_initialize(k_FunctionSymbol_t* symbol);

// Destructor

void k_FunctionSymbol_destroy(k_FunctionSymbol_t* symbol);

// Signature

/**
 * @memberof FunctionSymbol
 */
jtk_ArrayList_t* k_FunctionSymbol_getSignatures(k_FunctionSymbol_t* symbol);

/**
 * @memberof FunctionSymbol
 */
void k_FunctionSymbol_addSignature(k_FunctionSymbol_t* symbol, k_FunctionSignature_t* signature);

// Parameter Threshold

/**
 * @memberof FunctionSymbol
 */
int32_t k_FunctionSymbol_getParameterThreshold(k_FunctionSymbol_t* symbol);

/**
 * @memberof FunctionSymbol
 */
void k_FunctionSymbol_setParameterThreshold(k_FunctionSymbol_t* symbol, int32_t parameterThreshold);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_MEMBER_FUNCTION_SYMBOL_H */
