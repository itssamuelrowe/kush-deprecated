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

// Thursday, March 08, 2018

#include <kush/symbol-table/EnumerationSymbol.h>

/*******************************************************************************
 * EnumerationSymbol                                                                 *
 *******************************************************************************/

k_EnumerationSymbol_t* k_EnumerationSymbol_new(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope, k_Scope_t* enumerationScope) {
    k_EnumerationSymbol_t* enumerationSymbol = k_Memory_allocate(k_EnumerationSymbol_t, 1);

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_ENUMERATION, identifier, enclosingScope, enumerationSymbol);

    enumerationSymbol->m_symbol = symbol;
    enumerationSymbol->m_superclass = NULL;
    enumerationSymbol->m_enumerationScope = enumerationScope;

    return enumerationSymbol;
}

void k_EnumerationSymbol_delete(k_EnumerationSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    k_Symbol_delete(symbol->m_symbol);
    jtk_Memory_deallocate(symbol);
}

k_Scope_t* k_EnumerationSymbol_getEnumerationScope(k_EnumerationSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_enumerationScope;
}

k_ClassSymbol_t* k_EnumerationSymbol_getSuperclass(k_EnumerationSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_superclass;
}

k_Symbol_t* k_EnumerationSymbol_getSymbol(k_EnumerationSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_symbol;
}
