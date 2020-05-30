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

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H

#include <kush/Configuration.h>
#include <kush/Compiler.h>
#include <kush/ast/ASTListener.h>
#include <kush/ast/context/Context.h>
#include <kush/symbol-table/Scope.h>
#include <kush/symbol-table/ScopeType.h>
#include <kush/symbol-table/SymbolTable.h>
#include <kush/ast/ASTAnnotations.h>

/*******************************************************************************
 * SymbolDefinitionListener                                                    *
 *******************************************************************************/

/**
 * @class SymbolDefinitionListener
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_SymbolDefinitionListener_t {
    k_Compiler_t* m_compiler;
    k_ASTListener_t* m_astListener;
    k_SymbolTable_t* m_symbolTable;
    k_ASTAnnotations_t* m_scopes;
    const uint8_t* m_package;
    int32_t m_packageSize;
    k_ASTNodeType_t m_mainComponent;
    bool m_classPrepared;
    uint8_t* m_className;
    int32_t m_classNameSize;
};

/**
 * @memberof SymbolDefinitionListener
 */
typedef struct k_SymbolDefinitionListener_t k_SymbolDefinitionListener_t;

// Contructor

/**
 * @memberof SymbolDefinitionListener
 */
k_SymbolDefinitionListener_t* k_SymbolDefinitionListener_new(k_Compiler_t* compiler);

// Destructor

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_delete(k_SymbolDefinitionListener_t* listener);

// AST Listener

k_ASTListener_t* k_SymbolDefinitionListener_getASTListener(k_SymbolDefinitionListener_t* listener);

// Reset

void k_SymbolDefinitionListener_reset(
    k_SymbolDefinitionListener_t* symbolDefinitionListener,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes,
    const uint8_t* package, int32_t packageSize);

// Event Handlers

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterVariableDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterConstantDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterLabelClause(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterForParameters(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterTryStatement(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterClassDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitClassDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterEnumerationDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitEnumerationDeclaration(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onEnterEnumerate(k_ASTListener_t* astListener, k_ASTNode_t* node);

/**
 * @memberof SymbolDefinitionListener
 */
void k_SymbolDefinitionListener_onExitEnumerate(k_ASTListener_t* astListener, k_ASTNode_t* node);

k_Symbol_t* k_SymbolDefinitionListener_declareFunction(k_SymbolTable_t* symbolTable,
    k_ASTNode_t* identifier, jtk_ArrayList_t* fixedParameters,
    k_ASTNode_t* variableParameter, uint16_t modifiers);
void k_SymbolDefinitionListener_declareOverloadedFunction(
    k_SymbolDefinitionListener_t* listener, k_Symbol_t* symbol,
    jtk_ArrayList_t* fixedParameters, k_ASTNode_t* variableParameter,
    uint32_t modifiers, k_Token_t* reference);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H */
