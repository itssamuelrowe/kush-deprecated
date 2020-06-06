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

// Saturday, November 25, 2017

#ifndef KUSH_SCOPE_H
#define KUSH_SCOPE_H

#include <jtk/collection/list/ArrayList.h>
#include <jtk/collection/map/HashMap.h>

#include <kush/symbol-table/ScopeType.h>

typedef struct k_Symbol_t k_Symbol_t;

/*******************************************************************************
 * Scope                                                                       *
 *******************************************************************************/

/**
 * @memberof Scope
 */
typedef struct k_Scope_t k_Scope_t;

/**
 * @class Scope
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Scope_t {
    uint8_t* name;
    int32_t nameSize;
    k_ScopeType_t type;
    k_Scope_t* enclosingScope;
    jtk_HashMap_t* symbols;
    int32_t nextTicket;
    k_Symbol_t* symbol;
};

// Constructor

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_new(const uint8_t* name, int32_t nameSize,
    k_ScopeType_t type, k_Scope_t* enclosingScope, k_Symbol_t* symbol);

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_forCompilationUnit();

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_forFunction(k_Scope_t* enclosingScope);

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_forLocal(k_Scope_t* enclosingScope);

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_forClass(k_Scope_t* enclosingScope);

// Destructor

/**
 * @memberof Scope
 */
void k_Scope_delete(k_Scope_t* scope);

// Children Symbols

void k_Scope_getChildrenSymbols(k_Scope_t* scope, jtk_ArrayList_t* childrenSymbols);

// Context

void* k_Scope_getContext(k_Scope_t* scope);

// Scope Type

/**
 * @memberof Scope
 */
bool k_Scope_isEnumerationScope(k_Scope_t* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isClassScope(k_Scope_t* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isFunctionScope(k_Scope_t* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isCompilationUnitScope(k_Scope_t* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isLocalScope(k_Scope_t* scope);

// Define

/**
 * @memberof Scope
 */
void k_Scope_define(k_Scope_t* scope, k_Symbol_t* symbol);

/**
 * @memberof Scope
 */
void k_Scope_defineEx(k_Scope_t* scope, const uint8_t* descriptor,
    int32_t descriptorSize, k_Symbol_t* symbol);

// Enclosing Scope

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_getEnclosingScope(k_Scope_t* scope);

// Name

/**
 * @memberof Scope
 */
const uint8_t* k_Scope_getName(k_Scope_t* scope);

// Resolve

/**
 * @memberof Scope
 */
k_Symbol_t* k_Scope_resolve(k_Scope_t* scope, uint8_t* identifier);

/**
 * @memberof Scope
 */
k_Scope_t* k_Scope_resolveQualifiedSymbol(k_Scope_t* scope,
    const uint8_t* name, int32_t nameSize);

// Type

/**
 * @memberof Scope
 */
k_ScopeType_t k_Scope_getType(k_Scope_t* scope);

#endif /* KUSH_SCOPE_H */
