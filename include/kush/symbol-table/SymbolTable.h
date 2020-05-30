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

// Satuday, November 25, 2017

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_TABLE_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_TABLE_H

#include <kush/symbol-table/Scope.h>
#include <kush/symbol-table/Symbol.h>

struct k_Compiler_t;
typedef struct k_Compiler_t k_Compiler_t;

/*******************************************************************************
 * SymbolTable                                                                 *
 *******************************************************************************/

/**
 * @class SymbolTable
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_SymbolTable_t {
    k_Scope_t* m_currentScope;
    k_Compiler_t* m_compiler;
};

/**
 * @memberof SymbolTable
 */
typedef struct k_SymbolTable_t k_SymbolTable_t;

// Constructor

/**
 * @memberof SymbolTable
 */
k_SymbolTable_t* k_SymbolTable_new(k_Compiler_t* compiler);

// Destructor

/**
 * @memberof SymbolTable
 */
void k_SymbolTable_delete(k_SymbolTable_t* symbolTable);

// Current Scope

/**
 * @memberof SymbolTable
 */
void k_SymbolTable_setCurrentScope(k_SymbolTable_t* symbolTable, k_Scope_t* currentScope);

/**
 * @memberof SymbolTable
 */
k_Scope_t* k_SymbolTable_getCurrentScope(k_SymbolTable_t* symbolTable);

/**
 * @memberof SymbolTable
 */
void k_SymbolTable_invalidateCurrentScope(k_SymbolTable_t* symbolTable);

// Define

/**
 * @memberof SymbolTable
 */
void k_SymbolTable_define(k_SymbolTable_t* symbolTable, k_Symbol_t* symbol);

/**
 * @memberof SymbolTable
 */
void k_SymbolTable_defineClassMember(k_SymbolTable_t* symbolTable, k_Scope_t* classScope, k_Symbol_t* symbol);

// Resolve

/**
 * @memberof SymbolTable
 */
k_Symbol_t* k_SymbolTable_resolve(k_SymbolTable_t* symbolTable, const uint8_t* identifier);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_TABLE_H */
