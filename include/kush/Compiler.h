/*
 * Copyright 2018-2020 Samuel Rowe
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

// Monday, March 16, 2020

#ifndef KUSH_COMPILER_COMPILER_H
#define KUSH_COMPILER_COMPILER_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>
#include <kush/ast/ASTAnnotations.h>
#include <kush/symbol-table/SymbolTable.h>
#include <kush/symbol-table/SymbolLoader.h>
#include <kush/support/ErrorHandler.h>

#include <jtk/collection/list/ArrayList.h>
#include <jtk/collection/map/HashMap.h>
#include <jtk/log/Logger.h>

/******************************************************************************
 * Compiler                                                                   *
 ******************************************************************************/

/**
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Compiler_t {
    bool m_dumpTokens;
    bool m_dumpNodes;
    bool m_footprint;
    bool m_dumpInstructions;
    jtk_Logger_t* m_logger;
    jtk_ArrayList_t* m_inputFiles;
    int32_t m_currentFileIndex;
    k_ErrorHandler_t* m_errorHandler;
    k_ASTNode_t** m_compilationUnits;
    k_SymbolTable_t** m_symbolTables;
    k_ASTAnnotations_t** m_scopes;
    uint8_t** m_packages;
    int32_t* m_packageSizes;
    k_SymbolLoader_t* m_symbolLoader;
    jtk_HashMap_t* m_repository;
    jtk_ArrayList_t* m_trash;
    bool m_coreApi;
};

/**
 * @memberof Compiler
 */
typedef struct k_Compiler_t k_Compiler_t;

// Constructor

k_Compiler_t* k_Compiler_new();

// Destructor

void k_Compiler_delete(k_Compiler_t* compiler);

// Error

void k_Compiler_printErrors(k_Compiler_t* compiler);

// Phase

void k_Compiler_initialize(k_Compiler_t* compiler);
void k_Compiler_buildAST(k_Compiler_t* compiler);
void k_Compiler_analyze(k_Compiler_t* compiler);
void k_Compiler_generate(k_Compiler_t* compiler);
void k_Compiler_destroyNestedScopes(k_ASTAnnotations_t* annotations);
void k_Compiler_destroySymbol(k_Symbol_t* symbol);
void k_Compiler_destroyScope(k_Scope_t* scope);

// Register

void k_Compiler_registerSymbol(k_Compiler_t* compiler, const uint8_t* identifier,
    int32_t identifierSize, k_Symbol_t* symbol);

k_Symbol_t* k_Compiler_resolveSymbol(k_Compiler_t* compiler,
    const uint8_t* name, int32_t nameSize);

// Token

void k_Compiler_printToken(k_Token_t* token);
void k_Compiler_k_Compiler_printTokens(k_Compiler_t* compiler, jtk_ArrayList_t* tokens);

// Compiler

bool k_Compiler_compileEx(k_Compiler_t* compiler, char** arguments, int32_t length);
bool k_Compiler_compile(k_Compiler_t* compiler);

#endif /* KUSH_COMPILER_COMPILER_H */
