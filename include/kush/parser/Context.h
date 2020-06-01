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

// Sunday, May 31 2020

#ifndef KUSH_PARSER_CONTEXT_H
#define KUSH_PARSER_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

/*******************************************************************************
 * Context                                                                     *
 *******************************************************************************/

struct k_ASTNode_t {
    k_ASTNodeType_t m_type;
};

/*******************************************************************************
 * CompilationUnit                                                             *
 *******************************************************************************/

/**
 * @class CompilationUnit
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CompilationUnit_t {
    k_ASTNodeType_t m_type;
	jtk_ArrayList_t* m_imports;
	jtk_ArrayList_t* m_functions;
    jtk_ArrayList_t* m_structures;
};

/**
 * @memberof CompilationUnit
 */
typedef struct k_CompilationUnit_t k_CompilationUnit_t;

/*******************************************************************************
 * AdditiveExpression                                                          *
 *******************************************************************************/

/**
 * @class AdditiveExpression
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AdditiveExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_multiplicativeExpression;
    jtk_ArrayList_t* m_multiplicativeExpressions;
};

/**
 * @memberof AdditiveExpression
 */
typedef struct k_AdditiveExpression_t k_AdditiveExpression_t;

/*******************************************************************************
 * AndExpression                                                               *
 *******************************************************************************/

/**
 * @class AndExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AndExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_equalityExpression;
    jtk_ArrayList_t* m_equalityExpressions;
};

/**
 * @memberof AndExpression
 */
typedef struct k_AndExpression_t k_AndExpression_t;

/*******************************************************************************
 * VariableDeclaration                                                         *
 *******************************************************************************/

/**
 * @class VariableDeclaration

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_VariableDeclaration_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_declarators;
};

/**
 * @memberof VariableDeclaration
 */
typedef struct k_VariableDeclaration_t k_VariableDeclaration_t;

/*******************************************************************************
 * VariableDeclarator                                                          *
 *******************************************************************************/

/**
 * @class VariableDeclarator

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_VariableDeclarator_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof VariableDeclarator
 */
typedef struct k_VariableDeclarator_t k_VariableDeclarator_t;

/*******************************************************************************
 * TryStatement                                                                *
 *******************************************************************************/

/**
 * @class TryStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_TryStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_tryClause;
    jtk_ArrayList_t* m_catchClauses; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_finallyClause;
};

/**
 * @memberof TryStatement
 */
typedef struct k_TryStatement_t k_TryStatement_t;

/*******************************************************************************
 * TryClause                                                                   *
 *******************************************************************************/

/**
 * @class TryClause

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_TryClause_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof TryClause
 */
typedef struct k_TryClause_t k_TryClause_t;

/*******************************************************************************
 * ThrowStatement                                                              *
 *******************************************************************************/

/**
 * @class ThrowStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ThrowStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof ThrowStatement
 */
typedef struct k_ThrowStatement_t k_ThrowStatement_t;

/*******************************************************************************
 * ReturnStatement                                                             *
 *******************************************************************************/

/**
 * @class ReturnStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ReturnStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof ReturnStatement
 */
typedef struct k_ReturnStatement_t k_ReturnStatement_t;

/*******************************************************************************
 * StatementSuite                                                              *
 *******************************************************************************/

/**
 * @class StatementSuite

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_StatementSuite_t {
    k_ASTNodeType_t m_type;
    /* In order to help the users read code easily, the simple statements were
     * removed from statement suites. Previously, a statement suite was either
     * a simple statement or a block consisting of a newline at the begining,
     * indentation and dedentation.
     */
    // k_ASTNode_t* m_simpleStatement;
    jtk_ArrayList_t* m_statements;
    int32_t m_scope;
};

/**
 * @memberof StatementSuite
 */
typedef struct k_StatementSuite_t k_StatementSuite_t;

/*******************************************************************************
 * UnaryExpression                                                             *
 *******************************************************************************/

/**
 * @class UnaryExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_UnaryExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_unaryOperator;
    k_ASTNode_t* m_unaryExpression;
    k_ASTNode_t* m_postfixExpression;
};

/**
 * @memberof UnaryExpression
 */
typedef struct k_UnaryExpression_t k_UnaryExpression_t;

/*******************************************************************************
 * Subscript                                                                   *
 *******************************************************************************/

/**
 * @class Subscript

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Subscript_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof Subscript
 */
typedef struct k_Subscript_t k_Subscript_t;

/*******************************************************************************
 * ShiftExpression                                                             *
 *******************************************************************************/

/**
 * @class ShiftExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ShiftExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_additiveExpression;
    jtk_ArrayList_t* m_additiveExpressions;
};

/**
 * @memberof ShiftExpression
 */
typedef struct k_ShiftExpression_t k_ShiftExpression_t;

/*******************************************************************************
 * RelationalExpression                                                        *
 *******************************************************************************/

/**
 * @class RelationalExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_RelationalExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_shiftExpression;
    jtk_ArrayList_t* m_shiftExpressions;
};

/**
 * @memberof RelationalExpression
 */
typedef struct k_RelationalExpression_t k_RelationalExpression_t;

/*******************************************************************************
 * PrimaryExpression                                                           *
 *******************************************************************************/

/**
 * @class PrimaryExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PrimaryExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expression;
};

/*******************************************************************************
 * PostfixOperator                                                             *
 *******************************************************************************/

/**
 * @class PostfixOperator

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PostfixOperator_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_postfixOperator;
};

/**
 * @memberof PostfixOperator
 */
typedef struct k_PostfixOperator_t k_PostfixOperator_t;

/*******************************************************************************
 * PostfixExpression                                                           *
 *******************************************************************************/

/**
 * @class PostfixExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PostfixExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_primaryExpression;
    jtk_ArrayList_t* m_postfixParts; /* <k_ASTNode_t*> */
};

/**
 * @memberof PostfixExpression
 */
typedef struct k_PostfixExpression_t k_PostfixExpression_t;

/*******************************************************************************
 * MultiplicativeExpression                                                    *
 *******************************************************************************/

/**
 * @class MultiplicativeExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MultiplicativeExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_unaryExpression;
    jtk_ArrayList_t* m_unaryExpressions;
};

/**
 * @memberof MultiplicativeExpression
 */
typedef struct k_MultiplicativeExpression_t k_MultiplicativeExpression_t;

/*******************************************************************************
 * MemberAccess                                                                *
 *******************************************************************************/

/**
 * @class MemberAccess

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MemberAccess_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
};

/**
 * @memberof MemberAccess
 */
typedef struct k_MemberAccess_t k_MemberAccess_t;

/*******************************************************************************
 * MapExpression                                                               *
 *******************************************************************************/

/**
 * @class MapExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_mapEntries;
};

typedef struct k_MapExpression_t k_MapExpression_t;

/*******************************************************************************
 * MapEntry                                                                    *
 *******************************************************************************/

/**
 * @class MapEntry

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapEntry_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_keyExpression;
    k_ASTNode_t* m_valueExpression;
};

/**
 * @memberof MapEntry
 */
typedef struct k_MapEntry_t k_MapEntry_t;

/*******************************************************************************
 * MapEntries                                                                  *
 *******************************************************************************/

/**
 * @class MapEntries

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapEntries_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_mapEntries;
};

/**
 * @memberof MapEntries
 */
typedef struct k_MapEntries_t k_MapEntries_t;

/*******************************************************************************
 * LogicalOrExpression                                                         *
 *******************************************************************************/

/**
 * @class LogicalOrExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LogicalOrExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_logicalAndExpression;
    jtk_ArrayList_t* m_logicalAndExpressions;
};

/**
 * @memberof LogicalOrExpression
 */
typedef struct k_LogicalOrExpression_t k_LogicalOrExpression_t;

/*******************************************************************************
 * LogicalAndExpression                                                        *
 *******************************************************************************/

/**
 * @class LogicalAndExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LogicalAndExpression_t {

    k_ASTNodeType_t m_type;

    /**
     * The inclusiveOrExpression node on the left side of the operator.
     */
    k_ASTNode_t* m_inclusiveOrExpression;

    /**
     * The inclusiveOrExpression nodes on the right side of the operator.
     */
    jtk_ArrayList_t* m_inclusiveOrExpressions;
};

/**
 * @memberof LogicalAndExpression
 */
typedef struct k_LogicalAndExpression_t k_LogicalAndExpression_t;

/*******************************************************************************
 * ListExpression                                                              *
 *******************************************************************************/

/**
 * @class ListExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ListExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expressions;
};

typedef struct k_ListExpression_t k_ListExpression_t;

/*******************************************************************************
 * LabelClause                                                                 *
 *******************************************************************************/

// TODO: Use the ASTNode of the identifier instead of LabelClause       !

/**
 * @class LabelClause

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LabelClause_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
};

/**
 * @memberof LabelClause
 */
typedef struct k_LabelClause_t k_LabelClause_t;

/*******************************************************************************
 * IterativeStatement                                                          *
 *******************************************************************************/

/**
 * @class IterativeStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IterativeStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_labelClause;
    k_ASTNode_t* m_statement;
};

/**
 * @memberof IterativeStatement
 */
typedef struct k_IterativeStatement_t k_IterativeStatement_t;

/*******************************************************************************
 * InclusiveOrExpression                                                       *
 *******************************************************************************/

/**
 * @class InclusiveOrExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_InclusiveOrExpression_t {
    k_ASTNodeType_t m_type;
    /**
     * The expression on the left of the operator.
     */
    k_ASTNode_t* m_exclusiveOrExpression;

    /**
     * The list of exclusiveOrExpressions on the right of the operator.
     */
    jtk_ArrayList_t* m_exclusiveOrExpressions;
};

/**
 * @memberof InclusiveOrExpression
 */
typedef struct k_InclusiveOrExpression_t k_InclusiveOrExpression_t;

/*******************************************************************************
 * ImportDeclaration                                                           *
 *******************************************************************************/

/**
 * @class ImportDeclaration

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ImportDeclaration_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_identifiers; /* <k_ASTNode_t*> */
    bool m_wildcard;
};

/**
 * @memberof ImportDeclaration
 */
typedef struct k_ImportDeclaration_t k_ImportDeclaration_t;


/*******************************************************************************
 * IfStatement                                                                 *
 *******************************************************************************/

/**
 * @class IfStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IfStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_ifClause;
    jtk_ArrayList_t* m_elseIfClauses; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_elseClause;
};

/**
 * @memberof IfStatement
 */
typedef struct k_IfStatement_t k_IfStatement_t;


/*******************************************************************************
 * IfClause                                                                    *
 *******************************************************************************/

/**
 * @class IfClause

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IfClause_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expression;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof IfClause
 */
typedef struct k_IfClause_t k_IfClause_t;

/*******************************************************************************
 * FunctionParameters                                                          *
 *******************************************************************************/

/**
 * @class FunctionParameters

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionParameters_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_fixedParameters; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_variableParameter;
};

/**
 * @memberof FunctionParameters
 */
typedef struct k_FunctionParameters_t k_FunctionParameters_t;

/*******************************************************************************
 * FunctionDeclaration                                                         *
 *******************************************************************************/

/**
 * @class FunctionDeclaration

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionDeclaration_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_functionParameters;
    k_ASTNode_t* m_functionBody;
    k_Token_t* m_returnType;
    int32_t m_returnTypeDimensions;
};

/**
 * @memberof FunctionDeclaration
 */
typedef struct k_FunctionDeclaration_t k_FunctionDeclaration_t;

/*******************************************************************************
 * FunctionBody                                                                *
 *******************************************************************************/

/**
 * @class FunctionBody

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionBody_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof FunctionBody
 */
typedef struct k_FunctionBody_t k_FunctionBody_t;

/*******************************************************************************
 * FunctionArguments                                                           *
 *******************************************************************************/

/**
 * @class FunctionArguments

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionArguments_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_expressions;
};

/**
 * @memberof FunctionArguments
 */
typedef struct k_FunctionArguments_t k_FunctionArguments_t;

/*******************************************************************************
 * ForStatement                                                                *
 *******************************************************************************/

/**
 * @class ForStatement

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ForStatement_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_forParameter;
    k_ASTNode_t* m_expression;
    k_ASTNode_t* m_statementSuite;
    k_ASTNode_t* m_elseClause;
};

/**
 * @memberof ForStatement
 */
typedef struct k_ForStatement_t k_ForStatement_t;

/*******************************************************************************
 * ForParameter                                                               *
 *******************************************************************************/

/**
 * @class ForParameter

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ForParameter_t {
    k_ASTNodeType_t m_type;
    bool m_declaration;
    bool m_variable;
    k_Token_t* m_identifier;
};

/**
 * @memberof ForParameter
 */
typedef struct k_ForParameter_t k_ForParameter_t;

/*******************************************************************************
 * FinallyClause                                                               *
 *******************************************************************************/

/**
 * @class FinallyClause

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FinallyClause_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof FinallyClause
 */
typedef struct k_FinallyClause_t k_FinallyClause_t;

/*******************************************************************************
 * Expressions                                                                 *
 *******************************************************************************/

/**
 * @class Expressions

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Expressions_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_expressions; /* <k_ASTNode_t*> */
};

/**
 * @memberof Expressions
 */
typedef struct k_Expressions_t k_Expressions_t;

/*******************************************************************************
 * Expression                                                                  *
 *******************************************************************************/

/**
 * @class Expression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Expression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_assignmentExpression;
};

/**
 * @memberof Expression
 */
typedef struct k_Expression_t k_Expression_t;

/*******************************************************************************
 * ExclusiveOrExpression                                                       *
 *******************************************************************************/

/**
 * @class ExclusiveOrExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ExclusiveOrExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_andExpression;
    jtk_ArrayList_t* m_andExpressions;
};

/**
 * @memberof ExclusiveOrExpression
 */
typedef struct k_ExclusiveOrExpression_t k_ExclusiveOrExpression_t;

/*******************************************************************************
 * EqualityExpression                                                          *
 *******************************************************************************/

/**
 * @class EqualityExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_EqualityExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_relationalExpression;
    jtk_ArrayList_t* m_relationalExpressions;
};

/**
 * @memberof EqualityExpression
 */
typedef struct k_EqualityExpression_t k_EqualityExpression_t;

/*******************************************************************************
 * ConditionalExpression                                                       *
 *******************************************************************************/

/**
 * @class ConditionalExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ConditionalExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_logicalOrExpression;
    k_ASTNode_t* m_thenExpression;
    k_ASTNode_t* m_elseExpression;
};

/**
 * @memberof ConditionalExpression
 */
typedef struct k_ConditionalExpression_t k_ConditionalExpression_t;

/*******************************************************************************
 * ClassSuite                                                                  *
 *******************************************************************************/

/**
 * @class ClassSuite

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassSuite_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_classMembers; /* <k_ASTNode_t*> */
};

/**
 * @memberof ClassSuite
 */
typedef struct k_ClassSuite_t k_ClassSuite_t;

/*******************************************************************************
 * ClassMember                                                                 *
 *******************************************************************************/

/**
 * @class ClassMember

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassMember_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_annotations;
    jtk_ArrayList_t* m_modifiers; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_declaration;
};

/**
 * @memberof ClassMember
 */
typedef struct k_ClassMember_t k_ClassMember_t;

/*******************************************************************************
 * ClassDeclaration                                                            *
 *******************************************************************************/

/**
 * @class ClassDeclaration

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassDeclaration_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_structureSuite;
};

/**
 * @memberof ClassDeclaration
 */
typedef struct k_ClassDeclaration_t k_ClassDeclaration_t;

/*******************************************************************************
 * CatchFilter                                                                 *
 *******************************************************************************/

/**
 * @class CatchFilter

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CatchFilter_t {
    k_ASTNodeType_t m_type;
    jtk_ArrayList_t* m_typeNames; /* <k_ASTNode_t*> */
};

/**
 * @memberof CatchFilter
 */
typedef struct k_CatchFilter_t k_CatchFilter_t;

/*******************************************************************************
 * CatchClause                                                                 *
 *******************************************************************************/

/**
 * @class CatchClause

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CatchClause_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_catchFilter;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof CatchClause
 */
typedef struct k_CatchClause_t k_CatchClause_t;

/*******************************************************************************
 * BreakStatement                                                              *
 *******************************************************************************/

/**
 * @class BreakStatement
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_BreakStatement_t {
    k_ASTNodeType_t m_type;
    k_Token_t* m_identifier;
};

/**
 * @memberof BreakStatement
 */
typedef struct k_BreakStatement_t k_BreakStatement_t;

/*******************************************************************************
 * AssignmentExpression                                                        *
 *******************************************************************************/

/**
 * @class Assignme0ntExpression

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AssignmentExpression_t {
    k_ASTNodeType_t m_type;
    k_ASTNode_t* m_conditionalExpression;
    k_ASTNode_t* m_assignmentOperator;
    k_ASTNode_t* m_assignmentExpression;
};

/**
 * @memberof AssignmentExpression
 */
typedef struct k_AssignmentExpression_t k_AssignmentExpression_t;

struct k_FunctionParameter_t {
    k_Token_t* m_type;
    int32_t m_dimensions;
    k_Token_t* m_identifier;
    bool m_variable;
};

typedef struct k_FunctionParameter_t k_FunctionParameter_t;

struct k_StorageDeclarator_t {
    bool m_infer;
    bool m_constant;
    k_Token_t* m_typeName;
    int32_t m_dimensions;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_expression;
};

typedef struct k_StorageDeclarator_t k_StorageDeclarator_t;

// This is CatchContext
struct k_CatchFilter_t {
    jtk_ArrayList_t* m_captures;
    k_Token_t* m_identifier;
};

typedef struct k_CatchFilter_t k_CatchFilter_t;

#endif /* KUSH_PARSER_CONTEXT_H */