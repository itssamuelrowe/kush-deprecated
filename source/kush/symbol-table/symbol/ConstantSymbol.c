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

// Tuesday, February 27, 2018

#include <kush/symbol-table/ConstantSymbol.h>

/*******************************************************************************
 * ConstantSymbol                                                              *
 *******************************************************************************/

k_ConstantSymbol_t* k_ConstantSymbol_new(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope) {
    k_ConstantSymbol_t* constantSymbol = k_Memory_allocate(k_ConstantSymbol_t, 1);

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_CONSTANT, identifier, enclosingScope, constantSymbol);

    constantSymbol->m_symbol = symbol;
    constantSymbol->m_index = -1;

    return constantSymbol;
}

void k_ConstantSymbol_delete(k_ConstantSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    k_Symbol_delete(symbol->m_symbol);
    jtk_Memory_deallocate(symbol);
}

k_Symbol_t* k_ConstantSymbol_getSymbol(k_ConstantSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_symbol;
}