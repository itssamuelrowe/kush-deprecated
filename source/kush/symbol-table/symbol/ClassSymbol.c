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

#include <kush/symbol-table/ClassSymbol.h>
#include <jtk/core/CString.h>

/*******************************************************************************
 * ClassSymbol                                                                 *
 *******************************************************************************/

void k_ClassSymbol_initialize(k_ClassSymbol_t* symbol) {
    symbol->m_superClasses = jtk_ArrayList_new();
    symbol->m_classScope = NULL;
    symbol->m_qualifiedName = NULL; // jtk_CString_make(qualifiedName, &qualifiedNameSize);
    symbol->m_qualifiedNameSize = 0; // qualifiedNameSize;
}

void k_ClassSymbol_destroy(k_ClassSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    jtk_CString_delete(symbol->m_descriptor);
    jtk_CString_delete(symbol->m_qualifiedName);
    jtk_ArrayList_delete(symbol->m_superClasses);
}

k_Scope_t* k_ClassSymbol_getClassScope(k_ClassSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_classScope;
}

jtk_ArrayList_t* k_ClassSymbol_getSuperClasses(k_ClassSymbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return symbol->m_superClasses;
}

uint8_t* k_ClassSymbol_getQualifiedName(k_ClassSymbol_t* symbol) {
    /* To retrieve the fully qualified name a few resolution steps must be taken.
     * Therefore, a fully qualified name is evaluated and stored during the instantiation
     * of this class.
     *
     * Classes that are automatically made available by the compiler are represented
     * by the ImplicitClassSymbol class. This allows us to use the ASTNode and
     * be sure that the node originates from the source file. Therefore, we do
     * not worry about implicitly imported classes here.
     */
    return symbol->m_qualifiedName;
}