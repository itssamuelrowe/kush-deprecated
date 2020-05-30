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

// Saturday, March 10, 2018

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_RESOLUTION_LISTENER_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_RESOLUTION_LISTENER_H

#include <kush/Compiler.h>
#include <kush/ast/ASTAnnotations.h>
#include <kush/ast/ASTListener.h>
#include <kush/ast/ExpressionAnnotation.h>
#include <kush/symbol-table/SymbolTable.h>

/*******************************************************************************
 * SymbolResolutionListener                                                    *
 *******************************************************************************/

/**
 * @class SymbolResolutionListener
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_SymbolResolutionListener_t {
    k_Compiler_t* m_compiler;
    k_ASTListener_t* m_astListener;
    k_SymbolTable_t* m_symbolTable;
    k_ASTAnnotations_t* m_scopes;
    k_ExpressionAnnotation_t m_label;
    int32_t m_functionIndex;
    int32_t m_fieldIndex;
};

/**
 * @memberof SymbolResolutionListener
 */
typedef struct k_SymbolResolutionListener_t k_SymbolResolutionListener_t;

// Constructor

/**
 * @memberof SymbolResolutionListener
 */
k_SymbolResolutionListener_t* k_SymbolResolutionListener_new(k_Compiler_t* compiler);

// Destructor

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_delete(k_SymbolResolutionListener_t* listener);

// AST Listener

k_ASTListener_t* k_SymbolResolutionListener_getASTListener(k_SymbolResolutionListener_t* listener);

// Reset

void k_SymbolResolutionListener_reset(k_SymbolResolutionListener_t* listener,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes);

k_Symbol_t* k_SymbolResolutionListener_createExternalSymbol(
    k_SymbolResolutionListener_t* listener, k_Symbol_t* symbol,
    k_Scope_t* enclosingScope, k_ASTNode_t* identifier);

// Event Handlers

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onVisitErrorNode(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onVisitTerminal(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_SymbolResolutionListener_onEnterConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_SymbolResolutionListener_onExitConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEnumrationBaseClass(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEnumerationBaseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onExitMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_SymbolResolutionListener_onEnterMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_onExitMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_onEnterListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_onExitListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_onEnterNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof SymbolResolutionListener
 */
void k_SymbolResolutionListener_onExitNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_RESOLUTION_LISTENER_H */
