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

// Sunday, November 26, 2017

#ifndef KUSH_COMPILER_COMPILER_AST_LISTENER_H
#define KUSH_COMPILER_COMPILER_AST_LISTENER_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>
#include <kush/ast/ASTWalkerState.h>

/*******************************************************************************
 * ASTListener                                                                 *
 *******************************************************************************/

/**
 * @memberof ASTListener
 */
typedef struct k_ASTListener_t k_ASTListener_t;

/**
 * @memberof ASTListener
 */
typedef void (*k_OnVisitErrorNodeFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnVisitTerminalFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEveryRuleFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEveryRuleFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterCompilationUnitFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitCompilationUnitFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterImportDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitImportDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAnnotatedComponentDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAnnotatedComponentDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAnnotationsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAnnotationsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAnnotationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAnnotationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAnnotationTypeFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAnnotationTypeFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAnnotationAttributeFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAnnotationAttributeFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterComponentDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitComponentDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterFunctionDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitFunctionDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterFunctionParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitFunctionParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterFunctionBodyFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitFunctionBodyFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterStatementSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitStatementSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterSimpleStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitSimpleStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEmptyStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEmptyStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterVariableDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitVariableDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterVariableDeclaratorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitVariableDeclaratorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterConstantDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitConstantDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterConstantDeclaratorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitConstantDeclaratorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAssertStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAssertStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterBreakStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitBreakStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterContinueStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitContinueStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterReturnStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitReturnStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterThrowStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitThrowStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterCompoundStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitCompoundStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterIfStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitIfStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterIfClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitIfClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterElseIfClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitElseIfClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterElseClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitElseClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterIterativeStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitIterativeStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterLabelClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitLabelClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterWhileStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitWhileStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterForStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitForStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterForParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitForParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterTryStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitTryStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterTryClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitTryClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterCatchClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitCatchClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterCatchFilterFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitCatchFilterFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterFinallyClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitFinallyClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterSynchronizeStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitSynchronizeStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterWithStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitWithStatementFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterWithParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitWithParametersFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterWithParameterFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitWithParameterFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterClassDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitClassDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterClassExtendsClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitClassExtendsClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterClassSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitClassSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterClassMemberFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitClassMemberFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

// typedef void (*k_OnEnterConstructorDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);
// typedef void (*k_OnExitConstructorDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEnumerationDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEnumerationDeclarationFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEnumerationBaseClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEnumerationBaseClauseFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEnumerationSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEnumerationSuiteFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEnumerateFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEnumerateFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterExpressionsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitExpressionsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAssignmentExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAssignmentExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterConditionalExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitConditionalExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterLogicalOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitLogicalOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterLogialAndExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitLogicalAndExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterInclusiveOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitInclusiveOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterExclusiveOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitExclusiveOrExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAndExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAndExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterEqualityExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitEqualityExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterRelationalExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitRelationalExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterShiftExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitShiftExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterAdditiveExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitAdditiveExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterMultiplicativeExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitMultiplicativeExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterUnaryExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitUnaryExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterPostfixExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitPostfixExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterSubscriptFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitSubscriptFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterFunctionArgumentsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitFunctionArgumentsFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterMemberAccessFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitMemberAccessFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterPostfixOperatorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitPostfixOperatorFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterPrimaryExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitPrimaryExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterMapExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitMapExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterMapEntriesFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitMapEntriesFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterMapEntryFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitMapEntryFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterListExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitListExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnEnterNewExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
typedef void (*k_OnExitNewExpressionFunction_t)(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @class ASTListener
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ASTListener_t {
    k_OnVisitErrorNodeFunction_t m_onVisitErrorNode;
    k_OnVisitTerminalFunction_t m_onVisitTerminal;

    k_OnEnterEveryRuleFunction_t m_onEnterEveryRule;
    k_OnExitEveryRuleFunction_t m_onExitEveryRule;

    k_OnEnterCompilationUnitFunction_t m_onEnterCompilationUnit;
    k_OnExitCompilationUnitFunction_t m_onExitCompilationUnit;

    k_OnEnterImportDeclarationFunction_t m_onEnterImportDeclaration;
    k_OnExitImportDeclarationFunction_t m_onExitImportDeclaration;

    k_OnEnterAnnotatedComponentDeclarationFunction_t m_onEnterAnnotatedComponentDeclaration;
    k_OnExitAnnotatedComponentDeclarationFunction_t m_onExitAnnotatedComponentDeclaration;

    k_OnEnterAnnotationsFunction_t m_onEnterAnnotations;
    k_OnExitAnnotationsFunction_t m_onExitAnnotations;

    k_OnEnterAnnotationFunction_t m_onEnterAnnotation;
    k_OnExitAnnotationFunction_t m_onExitAnnotation;

    k_OnEnterAnnotationTypeFunction_t m_onEnterAnnotationType;
    k_OnExitAnnotationTypeFunction_t m_onExitAnnotationType;

    k_OnEnterAnnotationAttributeFunction_t m_onEnterAnnotationAttribute;
    k_OnExitAnnotationAttributeFunction_t m_onExitAnnotationAttribute;

    k_OnEnterComponentDeclarationFunction_t m_onEnterComponentDeclaration;
    k_OnExitComponentDeclarationFunction_t m_onExitComponentDeclaration;

    k_OnEnterFunctionDeclarationFunction_t m_onEnterFunctionDeclaration;
    k_OnExitFunctionDeclarationFunction_t m_onExitFunctionDeclaration;

    k_OnEnterFunctionParametersFunction_t m_onEnterFunctionParameters;
    k_OnExitFunctionParametersFunction_t m_onExitFunctionParameters;

    k_OnEnterFunctionBodyFunction_t m_onEnterFunctionBody;
    k_OnExitFunctionBodyFunction_t m_onExitFunctionBody;

    k_OnEnterStatementSuiteFunction_t m_onEnterStatementSuite;
    k_OnExitStatementSuiteFunction_t m_onExitStatementSuite;

    k_OnEnterSimpleStatementFunction_t m_onEnterSimpleStatement;
    k_OnExitSimpleStatementFunction_t m_onExitSimpleStatement;

    k_OnEnterStatementFunction_t m_onEnterStatement;
    k_OnExitStatementFunction_t m_onExitStatement;

    k_OnEnterEmptyStatementFunction_t m_onEnterEmptyStatement;
    k_OnExitEmptyStatementFunction_t m_onExitEmptyStatement;

    k_OnEnterVariableDeclarationFunction_t m_onEnterVariableDeclaration;
    k_OnExitVariableDeclarationFunction_t m_onExitVariableDeclaration;

    k_OnEnterVariableDeclaratorFunction_t m_onEnterVariableDeclarator;
    k_OnExitVariableDeclaratorFunction_t m_onExitVariableDeclarator;

    k_OnEnterConstantDeclarationFunction_t m_onEnterConstantDeclaration;
    k_OnExitConstantDeclarationFunction_t m_onExitConstantDeclaration;

    k_OnEnterConstantDeclaratorFunction_t m_onEnterConstantDeclarator;
    k_OnExitConstantDeclaratorFunction_t m_onExitConstantDeclarator;

    k_OnEnterAssertStatementFunction_t m_onEnterAssertStatement;
    k_OnExitAssertStatementFunction_t m_onExitAssertStatement;

    k_OnEnterBreakStatementFunction_t m_onEnterBreakStatement;
    k_OnExitBreakStatementFunction_t m_onExitBreakStatement;

    k_OnEnterContinueStatementFunction_t m_onEnterContinueStatement;
    k_OnExitContinueStatementFunction_t m_onExitContinueStatement;

    k_OnEnterReturnStatementFunction_t m_onEnterReturnStatement;
    k_OnExitReturnStatementFunction_t m_onExitReturnStatement;

    k_OnEnterThrowStatementFunction_t m_onEnterThrowStatement;
    k_OnExitThrowStatementFunction_t m_onExitThrowStatement;

    k_OnEnterCompoundStatementFunction_t m_onEnterCompoundStatement;
    k_OnExitCompoundStatementFunction_t m_onExitCompoundStatement;

    k_OnEnterIfStatementFunction_t m_onEnterIfStatement;
    k_OnExitIfStatementFunction_t m_onExitIfStatement;

    k_OnEnterIfClauseFunction_t m_onEnterIfClause;
    k_OnExitIfClauseFunction_t m_onExitIfClause;

    k_OnEnterElseIfClauseFunction_t m_onEnterElseIfClause;
    k_OnExitElseIfClauseFunction_t m_onExitElseIfClause;

    k_OnEnterElseClauseFunction_t m_onEnterElseClause;
    k_OnExitElseClauseFunction_t m_onExitElseClause;

    k_OnEnterIterativeStatementFunction_t m_onEnterIterativeStatement;
    k_OnExitIterativeStatementFunction_t m_onExitIterativeStatement;

    k_OnEnterLabelClauseFunction_t m_onEnterLabelClause;
    k_OnExitLabelClauseFunction_t m_onExitLabelClause;

    k_OnEnterWhileStatementFunction_t m_onEnterWhileStatement;
    k_OnExitWhileStatementFunction_t m_onExitWhileStatement;

    k_OnEnterForStatementFunction_t m_onEnterForStatement;
    k_OnExitForStatementFunction_t m_onExitForStatement;

    k_OnEnterForParametersFunction_t m_onEnterForParameter;
    k_OnExitForParametersFunction_t m_onExitForParameter;

    k_OnEnterTryStatementFunction_t m_onEnterTryStatement;
    k_OnExitTryStatementFunction_t m_onExitTryStatement;

    k_OnEnterTryClauseFunction_t m_onEnterTryClause;
    k_OnExitTryClauseFunction_t m_onExitTryClause;

    k_OnEnterCatchClauseFunction_t m_onEnterCatchClause;
    k_OnExitCatchClauseFunction_t m_onExitCatchClause;

    k_OnEnterCatchFilterFunction_t m_onEnterCatchFilter;
    k_OnExitCatchFilterFunction_t m_onExitCatchFilter;

    k_OnEnterFinallyClauseFunction_t m_onEnterFinallyClause;
    k_OnExitFinallyClauseFunction_t m_onExitFinallyClause;

    k_OnEnterSynchronizeStatementFunction_t m_onEnterSynchronizeStatement;
    k_OnExitSynchronizeStatementFunction_t m_onExitSynchronizeStatement;

    k_OnEnterWithStatementFunction_t m_onEnterWithStatement;
    k_OnExitWithStatementFunction_t m_onExitWithStatement;

    k_OnEnterWithParametersFunction_t m_onEnterWithParameters;
    k_OnExitWithParametersFunction_t m_onExitWithParameters;

    k_OnEnterWithParameterFunction_t m_onEnterWithParameter;
    k_OnExitWithParameterFunction_t m_onExitWithParameter;

    k_OnEnterClassDeclarationFunction_t m_onEnterClassDeclaration;
    k_OnExitClassDeclarationFunction_t m_onExitClassDeclaration;

    k_OnEnterClassExtendsClauseFunction_t m_onEnterClassExtendsClause;
    k_OnExitClassExtendsClauseFunction_t m_onExitClassExtendsClause;

    k_OnEnterClassSuiteFunction_t m_onEnterClassSuite;
    k_OnExitClassSuiteFunction_t m_onExitClassSuite;

    k_OnEnterClassMemberFunction_t m_onEnterClassMember;
    k_OnExitClassMemberFunction_t m_onExitClassMember;

//     k_OnEnterConstructorDeclarationFunction_t m_onEnterConstructorDeclaration;
//     k_OnExitConstructorDeclarationFunction_t m_onExitConstructorDeclaration;

    k_OnEnterEnumerationDeclarationFunction_t m_onEnterEnumerationDeclaration;
    k_OnExitEnumerationDeclarationFunction_t m_onExitEnumerationDeclaration;

    k_OnEnterEnumerationBaseClauseFunction_t m_onEnterEnumerationBaseClause;
    k_OnExitEnumerationBaseClauseFunction_t m_onExitEnumerationBaseClause;

    k_OnEnterEnumerationSuiteFunction_t m_onEnterEnumerationSuite;
    k_OnExitEnumerationSuiteFunction_t m_onExitEnumerationSuite;

    k_OnEnterEnumerateFunction_t m_onEnterEnumerate;
    k_OnExitEnumerateFunction_t m_onExitEnumerate;

    k_OnEnterExpressionsFunction_t m_onEnterExpressions;
    k_OnExitExpressionsFunction_t m_onExitExpressions;

    k_OnEnterExpressionFunction_t m_onEnterExpression;
    k_OnExitExpressionFunction_t m_onExitExpression;

    k_OnEnterAssignmentExpressionFunction_t m_onEnterAssignmentExpression;
    k_OnExitAssignmentExpressionFunction_t m_onExitAssignmentExpression;

    k_OnEnterConditionalExpressionFunction_t m_onEnterConditionalExpression;
    k_OnExitConditionalExpressionFunction_t m_onExitConditionalExpression;

    k_OnEnterLogicalOrExpressionFunction_t m_onEnterLogicalOrExpression;
    k_OnExitLogicalOrExpressionFunction_t m_onExitLogicalOrExpression;

    k_OnEnterLogialAndExpressionFunction_t m_onEnterLogicalAndExpression;
    k_OnExitLogicalAndExpressionFunction_t m_onExitLogicalAndExpression;

    k_OnEnterInclusiveOrExpressionFunction_t m_onEnterInclusiveOrExpression;
    k_OnExitInclusiveOrExpressionFunction_t m_onExitInclusiveOrExpression;

    k_OnEnterExclusiveOrExpressionFunction_t m_onEnterExclusiveOrExpression;
    k_OnExitExclusiveOrExpressionFunction_t m_onExitExclusiveOrExpression;

    k_OnEnterAndExpressionFunction_t m_onEnterAndExpression;
    k_OnExitAndExpressionFunction_t m_onExitAndExpression;

    k_OnEnterEqualityExpressionFunction_t m_onEnterEqualityExpression;
    k_OnExitEqualityExpressionFunction_t m_onExitEqualityExpression;

    k_OnEnterRelationalExpressionFunction_t m_onEnterRelationalExpression;
    k_OnExitRelationalExpressionFunction_t m_onExitRelationalExpression;

    k_OnEnterShiftExpressionFunction_t m_onEnterShiftExpression;
    k_OnExitShiftExpressionFunction_t m_onExitShiftExpression;

    k_OnEnterAdditiveExpressionFunction_t m_onEnterAdditiveExpression;
    k_OnExitAdditiveExpressionFunction_t m_onExitAdditiveExpression;

    k_OnEnterMultiplicativeExpressionFunction_t m_onEnterMultiplicativeExpression;
    k_OnExitMultiplicativeExpressionFunction_t m_onExitMultiplicativeExpression;

    k_OnEnterUnaryExpressionFunction_t m_onEnterUnaryExpression;
    k_OnExitUnaryExpressionFunction_t m_onExitUnaryExpression;

    k_OnEnterPostfixExpressionFunction_t m_onEnterPostfixExpression;
    k_OnExitPostfixExpressionFunction_t m_onExitPostfixExpression;

    k_OnEnterSubscriptFunction_t m_onEnterSubscript;
    k_OnExitSubscriptFunction_t m_onExitSubscript;

    k_OnEnterFunctionArgumentsFunction_t m_onEnterFunctionArguments;
    k_OnExitFunctionArgumentsFunction_t m_onExitFunctionArguments;

    k_OnEnterMemberAccessFunction_t m_onEnterMemberAccess;
    k_OnExitMemberAccessFunction_t m_onExitMemberAccess;

    k_OnEnterPostfixOperatorFunction_t m_onEnterPostfixOperator;
    k_OnExitPostfixOperatorFunction_t m_onExitPostfixOperator;

    k_OnEnterPrimaryExpressionFunction_t m_onEnterPrimaryExpression;
    k_OnExitPrimaryExpressionFunction_t m_onExitPrimaryExpression;

    k_OnEnterMapExpressionFunction_t m_onEnterMapExpression;
    k_OnExitMapExpressionFunction_t m_onExitMapExpression;

    k_OnEnterMapEntriesFunction_t m_onEnterMapEntries;
    k_OnExitMapEntriesFunction_t m_onExitMapEntries;

    k_OnEnterMapEntryFunction_t m_onEnterMapEntry;
    k_OnExitMapEntryFunction_t m_onExitMapEntry;

    k_OnEnterListExpressionFunction_t m_onEnterListExpression;
    k_OnExitListExpressionFunction_t m_onExitListExpression;

    k_OnEnterNewExpressionFunction_t m_onEnterNewExpression;
    k_OnExitNewExpressionFunction_t m_onExitNewExpression;

    k_ASTWalkerState_t m_walkerState;
    void* m_context;
};

/**
 * @memberof ASTListener
 */
typedef struct k_ASTListener_t k_ASTListener_t;

/**
 * @memberof ASTListener
 */
k_ASTListener_t* k_ASTListener_newWithContext(void* context);

/**
 * @memberof ASTListener
 */
void k_ASTListener_delete(k_ASTListener_t* listener);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onVisitErrorNode(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onVisitTerminal(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitElseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitLabel(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitForStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitForParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitTryClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterWithParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitWithParameters(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterWithParameter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitWithParameter(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitClassMember(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_ASTListener_onEnterConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);
// void k_ASTListener_onExitConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEnumrationBaseClass(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEnumerationBaseClause(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitExpressions(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitSubscript(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitListExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onEnterNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTListener
 */
void k_ASTListener_onExitNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node);

// Skip Children

void k_ASTListener_skipChildren(k_ASTListener_t* listener);

// Vist Children

void k_ASTListener_visitChildren(k_ASTListener_t* listener);

#endif /* KUSH_COMPILER_COMPILER_AST_LISTENER_H */
