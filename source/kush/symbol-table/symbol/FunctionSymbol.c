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

// Saturday, February 24, 2018

#include <kush/symbol-table/FunctionSymbol.h>

/*******************************************************************************
 * FunctionSymbol                                                              *
 *******************************************************************************/

void k_FunctionSymbol_initialize(k_FunctionSymbol_t* symbol) {
    symbol->m_signatures = jtk_ArrayList_new();
    symbol->m_parameterThreshold = -1;
}

void k_FunctionSymbol_destroy(k_FunctionSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    int32_t size = jtk_ArrayList_getSize(symbol->m_signatures);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_FunctionSignature_t* signature =
            (k_FunctionSignature_t*)jtk_ArrayList_getValue(symbol->m_signatures, i);
        k_FunctionSignature_delete(signature);
    }
    jtk_ArrayList_delete(symbol->m_signatures);
}

jtk_ArrayList_t* k_FunctionSymbol_getSignatures(k_FunctionSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_signatures;
}

void k_FunctionSymbol_addSignature(k_FunctionSymbol_t* symbol, k_FunctionSignature_t* signature) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    jtk_ArrayList_add(symbol->m_signatures, signature);
}

int32_t k_FunctionSymbol_getParameterThreshold(k_FunctionSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_parameterThreshold;
}

bool k_FunctionSymbol_hasParameterThreshold(k_FunctionSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    printf("has pt: %d\n", symbol->m_parameterThreshold != -1);
    return symbol->m_parameterThreshold != -1;
}

void k_FunctionSymbol_setParameterThreshold(k_FunctionSymbol_t* symbol,
    int32_t parameterThreshold) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    symbol->m_parameterThreshold = parameterThreshold;
}