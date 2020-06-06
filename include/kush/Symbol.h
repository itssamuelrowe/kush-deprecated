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

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_H

#include <kush/ast/ASTNode.h>
#include <kush/symbol-table/Scope.h>
#include <kush/symbol-table/SymbolCategory.h>
#include <kush/symbol-table/ClassSymbol.h>
#include <kush/symbol-table/FunctionSymbol.h>
#include <kush/symbol-table/Modifier.h>

enum k_SymbolFlag_t {
    KUSH_SYMBOL_FLAG_EXTERNAL = (1 << 0)
};

/*******************************************************************************
 * Symbol                                                                      *
 *******************************************************************************/

/**
 * @class Symbol
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Symbol_t {
    k_SymbolCategory_t category;
    k_ASTNode_t* identifier;
    k_Scope_t* enclosingScope;
    union {
        k_ClassSymbol_t asClass;
        k_FunctionSymbol_t asFunction;
        k_Symbol_t* asExternal;
    } context;
    uint32_t modifiers;
    int32_t ticket;
    int32_t index;
    uint32_t flags;

    uint8_t* name;
    int32_t nameSize;
};

/**
 * @memberof Symbol
 */
typedef struct k_Symbol_t k_Symbol_t;

// Constructor

/**
 * @memberof Symbol
 */
k_Symbol_t* k_Symbol_new(k_SymbolCategory_t category,
    k_ASTNode_t* identifier, k_Scope_t* enclosingScope);

k_Symbol_t* k_Symbol_forConstant(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope);

k_Symbol_t* k_Symbol_forVariable(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope);

k_Symbol_t* k_Symbol_forFunction(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope);

/**
 * This function is used for class declarations in compilation units.
 */
k_Symbol_t* k_Symbol_forClass(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope, k_Scope_t* classScope, const uint8_t* name,
    int32_t nameSize, const uint8_t* package, int32_t packageSize);

/**
 * This function is used by the symbol loader.
 */
k_Symbol_t* k_Symbol_forClassAlt(k_Scope_t* classScope, const uint8_t* descriptor,
    int32_t descriptorSize);

k_Symbol_t* k_Symbol_forLabel(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope);

k_Symbol_t* k_Symbol_forExternal(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope, k_Symbol_t* other);

// Destructor

/**
 * @memberof Symbol
 */
void k_Symbol_delete(k_Symbol_t* symbol);

// Category

k_SymbolCategory_t k_Symbol_getCategory(k_Symbol_t* symbol);

// Function

/**
 * @memberof Symbol
 */
bool k_Symbol_isFunction(k_Symbol_t* symbol);

// Constant

/**
 * @memberof Symbol
 */
bool k_Symbol_isConstant(k_Symbol_t* symbol);

// Enumerate

/**
 * @memberof Symbol
 */
bool k_Symbol_isEnumerate(k_Symbol_t* symbol);

// Variable

/**
 * @memberof Symbol
 */
bool k_Symbol_isVariable(k_Symbol_t* symbol);

// Enumeration

/**
 * @memberof Symbol
 */
bool k_Symbol_isEnumeration(k_Symbol_t* symbol);

// External

/**
 * @memberof Symbol
 */
bool k_Symbol_isExternal(k_Symbol_t* symbol);

bool k_Symbol_isStatic(k_Symbol_t* symbol);

// Enclosing Scope

/**
 * @memberof Symbol
 */
k_Scope_t* k_Symbol_getEnclosingScope(k_Symbol_t* symbol);

// Identifier

/**
 * @memberof Symbol
 */
k_ASTNode_t* k_Symbol_getIdentifier(k_Symbol_t* symbol);

void k_Symbol_addModifiers(k_Symbol_t* symbol, uint32_t modifiers);

k_FunctionSignature_t* k_Symbol_getFunctionSignatureEx(k_Symbol_t* symbol,
    const uint8_t* descriptor, int32_t descriptorSize);
k_FunctionSignature_t* k_Symbol_getFunctionSignature(k_Symbol_t* symbol,
    int32_t argumentCount);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_H */
