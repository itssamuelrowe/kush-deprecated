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

#include <kush/configuration.h>

typedef struct Context Context;

/*******************************************************************************
 * ScopeType                                                                   *
 *******************************************************************************/

/**
 * @class ScopeType
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
enum k_ScopeType_t {
    KUSH_SCOPE_COMPILATION_UNIT,
    KUSH_SCOPE_STRUCTURE,
    KUSH_SCOPE_FUNCTION,
    KUSH_SCOPE_LOCAL
};

/**
 * @memberof ScopeType
 */
typedef enum k_ScopeType_t k_ScopeType_t;

/*******************************************************************************
 * Scope                                                                       *
 *******************************************************************************/

/**
 * @memberof Scope
 */
typedef struct Scope Scope;

/**
 * @class Scope
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct Scope {
    uint8_t* name;
    int32_t nameSize;
    k_ScopeType_t type;
    Scope* parent;
    jtk_HashMap_t* symbols;
    int32_t nextTicket;
    Context* symbol;
};

// Constructor

/**
 * @memberof Scope
 */
Scope* k_Scope_new(const uint8_t* name, int32_t nameSize,
    k_ScopeType_t type, Scope* parent, Context* symbol);

/**
 * @memberof Scope
 */
Scope* scopeForModule();

/**
 * @memberof Scope
 */
Scope* scopeForFunction(Scope* parent);

/**
 * @memberof Scope
 */
Scope* scopeForLocal(Scope* parent);

/**
 * @memberof Scope
 */
Scope* k_Scope_forClass(Scope* parent);

// Destructor

/**
 * @memberof Scope
 */
void deleteScope(Scope* scope);

// Children Symbols

void getScopeChildren(Scope* scope, jtk_ArrayList_t* childrenSymbols);

// Context

void* k_Scope_getContext(Scope* scope);

// Scope Type

/**
 * @memberof Scope
 */
bool k_Scope_isEnumerationScope(Scope* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isClassScope(Scope* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isFunctionScope(Scope* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isCompilationUnitScope(Scope* scope);

/**
 * @memberof Scope
 */
bool k_Scope_isLocalScope(Scope* scope);

// Define

/**
 * @memberof Scope
 */
void k_Scope_define(Scope* scope, Context* symbol);

/**
 * @memberof Scope
 */
void k_Scope_defineEx(Scope* scope, const uint8_t* descriptor,
    int32_t descriptorSize, Context* symbol);

// Enclosing Scope

/**
 * @memberof Scope
 */
Scope* k_Scope_getEnclosingScope(Scope* scope);

// Name

/**
 * @memberof Scope
 */
const uint8_t* k_Scope_getName(Scope* scope);

// Resolve

/**
 * @memberof Scope
 */
Context* k_Scope_resolve(Scope* scope, uint8_t* identifier);

/**
 * @memberof Scope
 */
Scope* k_Scope_resolveQualifiedSymbol(Scope* scope,
    const uint8_t* name, int32_t nameSize);

// Type

/**
 * @memberof Scope
 */
k_ScopeType_t k_Scope_getType(Scope* scope);

#endif /* KUSH_SCOPE_H */
