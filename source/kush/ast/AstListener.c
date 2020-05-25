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

// Saturday, January 06, 2018

#include <kush/ast/ASTListener.h>

/*******************************************************************************
 * ASTListener                                                                 *
 *******************************************************************************/

k_ASTListener_t* k_ASTListener_newWithContext(void* context) {
    k_ASTListener_t* listener = k_Memory_allocate(k_ASTListener_t, 1);
    // listener->m_walkerState = ZEN_AST_WALKER_STATE_INVALID;
    listener->m_walkerState = ZEN_AST_WALKER_STATE_VISIT_CHILDREN;
    listener->m_context = context;

    listener->m_onVisitErrorNode = k_ASTListener_onVisitErrorNode;
    listener->m_onVisitTerminal = k_ASTListener_onVisitTerminal;

    listener->m_onEnterEveryRule = k_ASTListener_onEnterEveryRule;
    listener->m_onExitEveryRule = k_ASTListener_onExitEveryRule;

    listener->m_onEnterCompilationUnit = k_ASTListener_onEnterCompilationUnit;
    listener->m_onExitCompilationUnit = k_ASTListener_onExitCompilationUnit;

    listener->m_onEnterImportDeclaration = k_ASTListener_onEnterImportDeclaration;
    listener->m_onExitImportDeclaration = k_ASTListener_onExitImportDeclaration;

    listener->m_onEnterAnnotatedComponentDeclaration = k_ASTListener_onEnterAnnotatedComponentDeclaration;
    listener->m_onExitAnnotatedComponentDeclaration = k_ASTListener_onExitAnnotatedComponentDeclaration;

    listener->m_onEnterAnnotations = k_ASTListener_onEnterAnnotations;
    listener->m_onExitAnnotations = k_ASTListener_onExitAnnotations;

    listener->m_onEnterAnnotation = k_ASTListener_onEnterAnnotation;
    listener->m_onExitAnnotation = k_ASTListener_onExitAnnotation;

    listener->m_onEnterAnnotationType = k_ASTListener_onEnterAnnotationType;
    listener->m_onExitAnnotationType = k_ASTListener_onExitAnnotationType;

    listener->m_onEnterAnnotationAttribute = k_ASTListener_onEnterAnnotationAttribute;
    listener->m_onExitAnnotationAttribute = k_ASTListener_onExitAnnotationAttribute;

    listener->m_onEnterComponentDeclaration = k_ASTListener_onEnterComponentDeclaration;
    listener->m_onExitComponentDeclaration = k_ASTListener_onExitComponentDeclaration;

    listener->m_onEnterFunctionDeclaration = k_ASTListener_onEnterFunctionDeclaration;
    listener->m_onExitFunctionDeclaration = k_ASTListener_onExitFunctionDeclaration;

    listener->m_onEnterFunctionParameters = k_ASTListener_onEnterFunctionParameters;
    listener->m_onExitFunctionParameters = k_ASTListener_onExitFunctionParameters;

    listener->m_onEnterFunctionBody = k_ASTListener_onEnterFunctionBody;
    listener->m_onExitFunctionBody = k_ASTListener_onExitFunctionBody;

    listener->m_onEnterStatementSuite = k_ASTListener_onEnterStatementSuite;
    listener->m_onExitStatementSuite = k_ASTListener_onExitStatementSuite;

    listener->m_onEnterSimpleStatement = k_ASTListener_onEnterSimpleStatement;
    listener->m_onExitSimpleStatement = k_ASTListener_onExitSimpleStatement;

    listener->m_onEnterStatement = k_ASTListener_onEnterStatement;
    listener->m_onExitStatement = k_ASTListener_onExitStatement;

    listener->m_onEnterEmptyStatement = k_ASTListener_onEnterEmptyStatement;
    listener->m_onExitEmptyStatement = k_ASTListener_onExitEmptyStatement;

    listener->m_onEnterVariableDeclaration = k_ASTListener_onEnterVariableDeclaration;
    listener->m_onExitVariableDeclaration = k_ASTListener_onExitVariableDeclaration;

    listener->m_onEnterVariableDeclarator = k_ASTListener_onEnterVariableDeclarator;
    listener->m_onExitVariableDeclarator = k_ASTListener_onExitVariableDeclarator;

    listener->m_onEnterConstantDeclaration = k_ASTListener_onEnterConstantDeclaration;
    listener->m_onExitConstantDeclaration = k_ASTListener_onExitConstantDeclaration;

    listener->m_onEnterConstantDeclarator = k_ASTListener_onEnterConstantDeclarator;
    listener->m_onExitConstantDeclarator = k_ASTListener_onExitConstantDeclarator;

    listener->m_onEnterAssertStatement = k_ASTListener_onEnterAssertStatement;
    listener->m_onExitAssertStatement = k_ASTListener_onExitAssertStatement;

    listener->m_onEnterBreakStatement = k_ASTListener_onEnterBreakStatement;
    listener->m_onExitBreakStatement = k_ASTListener_onExitBreakStatement;

    listener->m_onEnterContinueStatement = k_ASTListener_onEnterContinueStatement;
    listener->m_onExitContinueStatement = k_ASTListener_onExitContinueStatement;

    listener->m_onEnterReturnStatement = k_ASTListener_onEnterReturnStatement;
    listener->m_onExitReturnStatement = k_ASTListener_onExitReturnStatement;

    listener->m_onEnterThrowStatement = k_ASTListener_onEnterThrowStatement;
    listener->m_onExitThrowStatement = k_ASTListener_onExitThrowStatement;

    listener->m_onEnterCompoundStatement = k_ASTListener_onEnterCompoundStatement;
    listener->m_onExitCompoundStatement = k_ASTListener_onExitCompoundStatement;

    listener->m_onEnterIfStatement = k_ASTListener_onEnterIfStatement;
    listener->m_onExitIfStatement = k_ASTListener_onExitIfStatement;

    listener->m_onEnterIfClause = k_ASTListener_onEnterIfClause;
    listener->m_onExitIfClause = k_ASTListener_onExitIfClause;

    listener->m_onEnterElseIfClause = k_ASTListener_onEnterElseIfClause;
    listener->m_onExitElseIfClause = k_ASTListener_onExitElseIfClause;

    listener->m_onEnterElseClause = k_ASTListener_onEnterElseClause;
    listener->m_onExitElseClause = k_ASTListener_onExitElseClause;

    listener->m_onEnterIterativeStatement = k_ASTListener_onEnterIterativeStatement;
    listener->m_onExitIterativeStatement = k_ASTListener_onExitIterativeStatement;

    listener->m_onEnterLabelClause = k_ASTListener_onEnterLabel;
    listener->m_onExitLabelClause = k_ASTListener_onExitLabel;

    listener->m_onEnterWhileStatement = k_ASTListener_onEnterWhileStatement;
    listener->m_onExitWhileStatement = k_ASTListener_onExitWhileStatement;

    listener->m_onEnterForStatement = k_ASTListener_onEnterForStatement;
    listener->m_onExitForStatement = k_ASTListener_onExitForStatement;

    listener->m_onEnterForParameter = k_ASTListener_onEnterForParameters;
    listener->m_onExitForParameter = k_ASTListener_onExitForParameters;

    listener->m_onEnterTryStatement = k_ASTListener_onEnterTryStatement;
    listener->m_onExitTryStatement = k_ASTListener_onExitTryStatement;

    listener->m_onEnterTryClause = k_ASTListener_onEnterTryClause;
    listener->m_onExitTryClause = k_ASTListener_onExitTryClause;

    listener->m_onEnterCatchClause = k_ASTListener_onEnterCatchClause;
    listener->m_onExitCatchClause = k_ASTListener_onExitCatchClause;

    listener->m_onEnterCatchFilter = k_ASTListener_onEnterCatchFilter;
    listener->m_onExitCatchFilter = k_ASTListener_onExitCatchFilter;

    listener->m_onEnterFinallyClause = k_ASTListener_onEnterFinallyClause;
    listener->m_onExitFinallyClause = k_ASTListener_onExitFinallyClause;

    listener->m_onEnterSynchronizeStatement = k_ASTListener_onEnterSynchronizeStatement;
    listener->m_onExitSynchronizeStatement = k_ASTListener_onExitSynchronizeStatement;

    listener->m_onEnterWithStatement = k_ASTListener_onEnterWithStatement;
    listener->m_onExitWithStatement = k_ASTListener_onExitWithStatement;

    listener->m_onEnterWithParameters = k_ASTListener_onEnterWithParameters;
    listener->m_onExitWithParameters = k_ASTListener_onExitWithParameters;

    listener->m_onEnterWithParameter = k_ASTListener_onEnterWithParameter;
    listener->m_onExitWithParameter = k_ASTListener_onExitWithParameter;

    listener->m_onEnterClassDeclaration = k_ASTListener_onEnterClassDeclaration;
    listener->m_onExitClassDeclaration = k_ASTListener_onExitClassDeclaration;

    listener->m_onEnterClassExtendsClause = k_ASTListener_onEnterClassExtendsClause;
    listener->m_onExitClassExtendsClause = k_ASTListener_onExitClassExtendsClause;

    listener->m_onEnterClassSuite = k_ASTListener_onEnterClassSuite;
    listener->m_onExitClassSuite = k_ASTListener_onExitClassSuite;

    listener->m_onEnterClassMember = k_ASTListener_onEnterClassMember;
    listener->m_onExitClassMember = k_ASTListener_onExitClassMember;

    // listener->m_onEnterConstructorDeclaration = k_ASTListener_onEnterConstructorDeclaration;
    // listener->m_onExitConstructorDeclaration = k_ASTListener_onExitConstructorDeclaration;

    listener->m_onEnterEnumerationDeclaration = k_ASTListener_onEnterEnumerationDeclaration;
    listener->m_onExitEnumerationDeclaration = k_ASTListener_onExitEnumerationDeclaration;

    listener->m_onEnterEnumerationBaseClause = k_ASTListener_onEnterEnumrationBaseClass;
    listener->m_onExitEnumerationBaseClause = k_ASTListener_onExitEnumerationBaseClause;

    listener->m_onEnterEnumerationSuite = k_ASTListener_onEnterEnumerationSuite;
    listener->m_onExitEnumerationSuite = k_ASTListener_onExitEnumerationSuite;

    listener->m_onEnterEnumerate = k_ASTListener_onEnterEnumerate;
    listener->m_onExitEnumerate = k_ASTListener_onExitEnumerate;

    listener->m_onEnterExpressions = k_ASTListener_onEnterExpressions;
    listener->m_onExitExpressions = k_ASTListener_onExitExpressions;

    listener->m_onEnterExpression = k_ASTListener_onEnterExpression;
    listener->m_onExitExpression = k_ASTListener_onExitExpression;

    listener->m_onEnterAssignmentExpression = k_ASTListener_onEnterAssignmentExpression;
    listener->m_onExitAssignmentExpression = k_ASTListener_onExitAssignmentExpression;

    listener->m_onEnterConditionalExpression = k_ASTListener_onEnterConditionalExpression;
    listener->m_onExitConditionalExpression = k_ASTListener_onExitConditionalExpression;

    listener->m_onEnterLogicalOrExpression = k_ASTListener_onEnterLogicalOrExpression;
    listener->m_onExitLogicalOrExpression = k_ASTListener_onExitLogicalOrExpression;

    listener->m_onEnterLogicalAndExpression = k_ASTListener_onEnterLogicalAndExpression;
    listener->m_onExitLogicalAndExpression = k_ASTListener_onExitLogicalAndExpression;

    listener->m_onEnterInclusiveOrExpression = k_ASTListener_onEnterInclusiveOrExpression;
    listener->m_onExitInclusiveOrExpression = k_ASTListener_onExitInclusiveOrExpression;

    listener->m_onEnterExclusiveOrExpression = k_ASTListener_onEnterExclusiveOrExpression;
    listener->m_onExitExclusiveOrExpression = k_ASTListener_onExitExclusiveOrExpression;

    listener->m_onEnterAndExpression = k_ASTListener_onEnterAndExpression;
    listener->m_onExitAndExpression = k_ASTListener_onExitAndExpression;

    listener->m_onEnterEqualityExpression = k_ASTListener_onEnterEqualityExpression;
    listener->m_onExitEqualityExpression = k_ASTListener_onExitEqualityExpression;

    listener->m_onEnterRelationalExpression = k_ASTListener_onEnterRelationalExpression;
    listener->m_onExitRelationalExpression = k_ASTListener_onExitRelationalExpression;

    listener->m_onEnterShiftExpression = k_ASTListener_onEnterShiftExpression;
    listener->m_onExitShiftExpression = k_ASTListener_onExitShiftExpression;

    listener->m_onEnterAdditiveExpression = k_ASTListener_onEnterAdditiveExpression;
    listener->m_onExitAdditiveExpression = k_ASTListener_onExitAdditiveExpression;

    listener->m_onEnterMultiplicativeExpression = k_ASTListener_onEnterMultiplicativeExpression;
    listener->m_onExitMultiplicativeExpression = k_ASTListener_onExitMultiplicativeExpression;

    listener->m_onEnterUnaryExpression = k_ASTListener_onEnterUnaryExpression;
    listener->m_onExitUnaryExpression = k_ASTListener_onExitUnaryExpression;

    listener->m_onEnterPostfixExpression = k_ASTListener_onEnterPostfixExpression;
    listener->m_onExitPostfixExpression = k_ASTListener_onExitPostfixExpression;

    listener->m_onEnterSubscript = k_ASTListener_onEnterSubscript;
    listener->m_onExitSubscript = k_ASTListener_onExitSubscript;

    listener->m_onEnterFunctionArguments = k_ASTListener_onEnterFunctionArguments;
    listener->m_onExitFunctionArguments = k_ASTListener_onExitFunctionArguments;

    listener->m_onEnterMemberAccess = k_ASTListener_onEnterMemberAccess;
    listener->m_onExitMemberAccess = k_ASTListener_onExitMemberAccess;

    listener->m_onEnterPostfixOperator = k_ASTListener_onEnterPostfixOperator;
    listener->m_onExitPostfixOperator = k_ASTListener_onExitPostfixOperator;

    listener->m_onEnterPrimaryExpression = k_ASTListener_onEnterPrimaryExpression;
    listener->m_onExitPrimaryExpression = k_ASTListener_onExitPrimaryExpression;

    listener->m_onEnterMapExpression = k_ASTListener_onEnterMapExpression;
    listener->m_onExitMapExpression = k_ASTListener_onExitMapExpression;

    listener->m_onEnterMapEntries = k_ASTListener_onEnterMapEntries;
    listener->m_onExitMapEntries = k_ASTListener_onExitMapEntries;

    listener->m_onEnterMapEntry = k_ASTListener_onEnterMapEntry;
    listener->m_onExitMapEntry = k_ASTListener_onExitMapEntry;

    listener->m_onEnterListExpression = k_ASTListener_onEnterListExpression;
    listener->m_onExitListExpression = k_ASTListener_onExitListExpression;

    listener->m_onEnterNewExpression = k_ASTListener_onEnterNewExpression;
    listener->m_onExitNewExpression = k_ASTListener_onExitNewExpression;

    return listener;
}

void k_ASTListener_delete(k_ASTListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");
    jtk_Memory_deallocate(listener);
}

void k_ASTListener_onVisitErrorNode(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onVisitTerminal(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onEnterEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// compilationUnit

void k_ASTListener_onEnterCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitCompilationUnit(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// importDeclaration

void k_ASTListener_onEnterImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitImportDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// annotatedComponentDeclaration

void k_ASTListener_onEnterAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAnnotatedComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// annotations

void k_ASTListener_onEnterAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAnnotations(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// annotation

void k_ASTListener_onEnterAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAnnotation(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// annotationType

void k_ASTListener_onEnterAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAnnotationType(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// annotationAttribute

void k_ASTListener_onEnterAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAnnotationAttribute(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// componentDeclaration

void k_ASTListener_onEnterComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitComponentDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// functionDeclaration

void k_ASTListener_onEnterFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitFunctionDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// functionParameters

void k_ASTListener_onEnterFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitFunctionParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// functionBody

void k_ASTListener_onEnterFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitFunctionBody(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// statementSuite

void k_ASTListener_onEnterStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitStatementSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// simpleStatement

void k_ASTListener_onEnterSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitSimpleStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// statement

void k_ASTListener_onEnterStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// emptyStatement

void k_ASTListener_onEnterEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEmptyStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// variableDeclaration

void k_ASTListener_onEnterVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitVariableDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// variableDeclarator

void k_ASTListener_onEnterVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitVariableDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// constantDeclaration

void k_ASTListener_onEnterConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitConstantDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// constantDeclarator

void k_ASTListener_onEnterConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitConstantDeclarator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// assertStatement

void k_ASTListener_onEnterAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAssertStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// breakStatement

void k_ASTListener_onEnterBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitBreakStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// continueStatement

void k_ASTListener_onEnterContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitContinueStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// returnStatement

void k_ASTListener_onEnterReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitReturnStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// throwStatement

void k_ASTListener_onEnterThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitThrowStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// compoundStatement

void k_ASTListener_onEnterCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitCompoundStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// ifStatement

void k_ASTListener_onEnterIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitIfStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// ifClause

void k_ASTListener_onEnterIfClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitIfClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// elseIfClause

void k_ASTListener_onEnterElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitElseIfClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// elseClause

void k_ASTListener_onEnterElseClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitElseClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// iterativeStatement

void k_ASTListener_onEnterIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitIterativeStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// label

void k_ASTListener_onEnterLabel(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitLabel(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// whileStatement

void k_ASTListener_onEnterWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitWhileStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// forStatement

void k_ASTListener_onEnterForStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitForStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// forParameters

void k_ASTListener_onEnterForParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitForParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// tryStatement

void k_ASTListener_onEnterTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitTryStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// tryClause

void k_ASTListener_onEnterTryClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitTryClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// catchClause

void k_ASTListener_onEnterCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitCatchClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// catchFilter

void k_ASTListener_onEnterCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitCatchFilter(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// finallyClause

void k_ASTListener_onEnterFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitFinallyClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// synchronizeStatement

void k_ASTListener_onEnterSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitSynchronizeStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// withStatement

void k_ASTListener_onEnterWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitWithStatement(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// withParameters

void k_ASTListener_onEnterWithParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitWithParameters(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// withParameter

void k_ASTListener_onEnterWithParameter(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitWithParameter(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// classDeclaration

void k_ASTListener_onEnterClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitClassDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// classExtends

void k_ASTListener_onEnterClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitClassExtendsClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// classSuite

void k_ASTListener_onEnterClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitClassSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// classMember

void k_ASTListener_onEnterClassMember(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitClassMember(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// void k_ASTListener_onEnterConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
// }

// void k_ASTListener_onExitConstructorDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
// }


// enumerationDeclaration

void k_ASTListener_onEnterEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEnumerationDeclaration(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// enumerationBaseClass

void k_ASTListener_onEnterEnumrationBaseClass(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEnumerationBaseClause(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// enumerationSuite

void k_ASTListener_onEnterEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEnumerationSuite(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// enumerate

void k_ASTListener_onEnterEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEnumerate(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// expressions

void k_ASTListener_onEnterExpressions(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitExpressions(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// expression

void k_ASTListener_onEnterExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// assignmentExpression

void k_ASTListener_onEnterAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAssignmentExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// conditionalExpression

void k_ASTListener_onEnterConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitConditionalExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// logicalOrExpression

void k_ASTListener_onEnterLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitLogicalOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// logicalAndExpression

void k_ASTListener_onEnterLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitLogicalAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// inclusiveOrExpression

void k_ASTListener_onEnterInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitInclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// exclusiveOrExpression

void k_ASTListener_onEnterExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitExclusiveOrExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// andExpression

void k_ASTListener_onEnterAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAndExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// equalityExpression

void k_ASTListener_onEnterEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitEqualityExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// relationalExpression

void k_ASTListener_onEnterRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitRelationalExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// shiftExpression

void k_ASTListener_onEnterShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitShiftExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// additiveExpression

void k_ASTListener_onEnterAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitAdditiveExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// multiplicativeExpression

void k_ASTListener_onEnterMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitMultiplicativeExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// unaryExpression

void k_ASTListener_onEnterUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitUnaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// postfixExpression

void k_ASTListener_onEnterPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitPostfixExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// subscript

void k_ASTListener_onEnterSubscript(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitSubscript(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// functionArguments

void k_ASTListener_onEnterFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitFunctionArguments(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// memberAccess

void k_ASTListener_onEnterMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitMemberAccess(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// postfixOperator

void k_ASTListener_onEnterPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitPostfixOperator(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// primaryExpression

void k_ASTListener_onEnterPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitPrimaryExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// mapExpression

void k_ASTListener_onEnterMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitMapExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// mapEntries

void k_ASTListener_onEnterMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitMapEntries(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// mapEntry

void k_ASTListener_onEnterMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitMapEntry(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// listExpression

void k_ASTListener_onEnterListExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitListExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onEnterNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

void k_ASTListener_onExitNewExpression(k_ASTListener_t* listener, k_ASTNode_t* node) {
}

// Children

void k_ASTListener_skipChildren(k_ASTListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    listener->m_walkerState = ZEN_AST_WALKER_STATE_SKIP_CHILDREN;
}

void k_ASTListener_visitChildren(k_ASTListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    listener->m_walkerState = ZEN_AST_WALKER_STATE_VISIT_CHILDREN;
}

// First Child

void k_ASTListener_visitFirstChild(k_ASTListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    listener->m_walkerState = ZEN_AST_WALKER_STATE_VISIT_FIRST_CHILD;
}