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
 * CompilationUnitContext                                                      *
 *******************************************************************************/

/**
 * @class CompilationUnitContext
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CompilationUnit_t {
    /**
     * The node which represents this rule context within the AST.
     */

	jtk_ArrayList_t* m_imports;
	jtk_ArrayList_t* m_functions;
    jtk_ArrayList_t* m_structures;
};

/**
 * @memberof CompilationUnitContext
 */
typedef struct k_CompilationUnit_t k_CompilationUnit_t;

/*******************************************************************************
 * AdditiveExpressionContext                                                   *
 *******************************************************************************/

/**
 * @class AdditiveExpressionContext
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AdditiveExpression_t {

    k_ASTNode_t* m_multiplicativeExpression;
    jtk_ArrayList_t* m_multiplicativeExpressions;
};

/**
 * @memberof AdditiveExpressionContext
 */
typedef struct k_AdditiveExpression_t k_AdditiveExpression_t;

/*******************************************************************************
 * AndExpressionContext                                                        *
 *******************************************************************************/

/**
 * @class AndExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AndExpression_t {

    k_ASTNode_t* m_equalityExpression;
    jtk_ArrayList_t* m_equalityExpressions;
};

/**
 * @memberof AndExpressionContext
 */
typedef struct k_AndExpression_t k_AndExpression_t;

/*******************************************************************************
 * VariableDeclarationContext                                                  *
 *******************************************************************************/

/**
 * @class VariableDeclarationContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_VariableDeclaration_t {

    jtk_ArrayList_t* m_declarators;
};

/**
 * @memberof VariableDeclarationContext
 */
typedef struct k_VariableDeclaration_t k_VariableDeclaration_t;

/*******************************************************************************
 * VariableDeclaratorContext                                                   *
 *******************************************************************************/

/**
 * @class VariableDeclaratorContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_VariableDeclarator_t {

    k_Token_t* m_identifier;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof VariableDeclaratorContext
 */
typedef struct k_VariableDeclarator_t k_VariableDeclarator_t;

/*******************************************************************************
 * TryStatementContext                                                         *
 *******************************************************************************/

/**
 * @class TryStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_TryStatement_t {

    k_ASTNode_t* m_tryClause;
    jtk_ArrayList_t* m_catchClauses; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_finallyClause;
};

/**
 * @memberof TryStatementContext
 */
typedef struct k_TryStatement_t k_TryStatement_t;

/*******************************************************************************
 * TryClauseContext                                                            *
 *******************************************************************************/

/**
 * @class TryClauseContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_TryClause_t {

    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof TryClauseContext
 */
typedef struct k_TryClause_t k_TryClause_t;

/*******************************************************************************
 * ThrowStatementContext                                                       *
 *******************************************************************************/

/**
 * @class ThrowStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ThrowStatement_t {

    k_ASTNode_t* m_expression;
};

/**
 * @memberof ThrowStatementContext
 */
typedef struct k_ThrowStatement_t k_ThrowStatement_t;

/*******************************************************************************
 * ReturnStatementContext                                                      *
 *******************************************************************************/

/**
 * @class ReturnStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ReturnStatement_t {

    k_ASTNode_t* m_expression;
};

/**
 * @memberof ReturnStatementContext
 */
typedef struct k_ReturnStatement_t k_ReturnStatement_t;

/*******************************************************************************
 * StatementSuiteContext                                                       *
 *******************************************************************************/

/**
 * @class StatementSuiteContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_StatementSuite_t {

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
 * @memberof StatementSuiteContext
 */
typedef struct k_StatementSuite_t k_StatementSuite_t;

/*******************************************************************************
 * UnaryExpressionContext                                                      *
 *******************************************************************************/

/**
 * @class UnaryExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_UnaryExpression_t {

    k_ASTNode_t* m_unaryOperator;
    k_ASTNode_t* m_unaryExpression;
    k_ASTNode_t* m_postfixExpression;
};

/**
 * @memberof UnaryExpressionContext
 */
typedef struct k_UnaryExpression_t k_UnaryExpression_t;

/*******************************************************************************
 * SubscriptContext                                                            *
 *******************************************************************************/

/**
 * @class SubscriptContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Subscript_t {

    k_ASTNode_t* m_expression;
};

/**
 * @memberof SubscriptContext
 */
typedef struct k_Subscript_t k_Subscript_t;

/*******************************************************************************
 * ShiftExpressionContext                                                      *
 *******************************************************************************/

/**
 * @class ShiftExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ShiftExpression_t {

    k_ASTNode_t* m_additiveExpression;
    jtk_ArrayList_t* m_additiveExpressions;
};

/**
 * @memberof ShiftExpressionContext
 */
typedef struct k_ShiftExpression_t k_ShiftExpression_t;

/*******************************************************************************
 * RelationalExpressionContext                                                 *
 *******************************************************************************/

/**
 * @class RelationalExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_RelationalExpression_t {

    k_ASTNode_t* m_shiftExpression;
    jtk_ArrayList_t* m_shiftExpressions;
};

/**
 * @memberof RelationalExpressionContext
 */
typedef struct k_RelationalExpression_t k_RelationalExpression_t;

/*******************************************************************************
 * PrimaryExpressionContext                                                    *
 *******************************************************************************/

/**
 * @class PrimaryExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PrimaryExpression_t {

    k_ASTNode_t* m_expression;
};

/*******************************************************************************
 * PostfixOperatorContext                                                      *
 *******************************************************************************/

/**
 * @class PostfixOperatorContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PostfixOperator_t {

    k_ASTNode_t* m_postfixOperator;
};

/**
 * @memberof PostfixOperatorContext
 */
typedef struct k_PostfixOperator_t k_PostfixOperator_t;

/**
 * @memberof PostfixOperatorContext
 */
k_PostfixOperator_t* k_PostfixOperatorContext_new(k_ASTNode_t* node);

/**
 * @memberof PostfixOperatorContext
 */
void k_PostfixOperatorContext_delete(k_PostfixOperator_t* context);

/*******************************************************************************
 * PostfixExpressionContext                                                    *
 *******************************************************************************/

/**
 * @class PostfixExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_PostfixExpression_t {

    k_ASTNode_t* m_primaryExpression;
    jtk_ArrayList_t* m_postfixParts; /* <k_ASTNode_t*> */
};

/**
 * @memberof PostfixExpressionContext
 */
typedef struct k_PostfixExpression_t k_PostfixExpression_t;

/*******************************************************************************
 * MultiplicativeExpressionContext                                             *
 *******************************************************************************/

/**
 * @class MultiplicativeExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MultiplicativeExpression_t {

    k_ASTNode_t* m_unaryExpression;
    jtk_ArrayList_t* m_unaryExpressions;
};

/**
 * @memberof MultiplicativeExpressionContext
 */
typedef struct k_MultiplicativeExpression_t k_MultiplicativeExpression_t;

/*******************************************************************************
 * MemberAccessContext                                                         *
 *******************************************************************************/

/**
 * @class MemberAccessContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MemberAccess_t {

    k_Token_t* m_identifier;
};

/**
 * @memberof MemberAccessContext
 */
typedef struct k_MemberAccess_t k_MemberAccess_t;

/*******************************************************************************
 * MapExpressionContext                                                        *
 *******************************************************************************/

/**
 * @class MapExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapExpression_t {

    k_ASTNode_t* m_mapEntries;
};

typedef struct k_MapExpression_t k_MapExpression_t;

/*******************************************************************************
 * MapEntryContext                                                             *
 *******************************************************************************/

/**
 * @class MapEntryContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapEntry_t {

    k_ASTNode_t* m_keyExpression;
    k_ASTNode_t* m_valueExpression;
};

/**
 * @memberof MapEntryContext
 */
typedef struct k_MapEntry_t k_MapEntry_t;

/*******************************************************************************
 * MapEntriesContext                                                           *
 *******************************************************************************/

/**
 * @class MapEntriesContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_MapEntries_t {

    jtk_ArrayList_t* m_mapEntries;
};

/**
 * @memberof MapEntriesContext
 */
typedef struct k_MapEntries_t k_MapEntries_t;

/*******************************************************************************
 * LogicalOrExpressionContext                                                  *
 *******************************************************************************/

/**
 * @class LogicalOrExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LogicalOrExpression_t {

    k_ASTNode_t* m_logicalAndExpression;
    jtk_ArrayList_t* m_logicalAndExpressions;
};

/**
 * @memberof LogicalOrExpressionContext
 */
typedef struct k_LogicalOrExpression_t k_LogicalOrExpression_t;

/*******************************************************************************
 * LogicalAndExpressionContext                                                 *
 *******************************************************************************/

/**
 * @class LogicalAndExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LogicalAndExpression_t {


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
 * @memberof LogicalAndExpressionContext
 */
typedef struct k_LogicalAndExpression_t k_LogicalAndExpression_t;

/*******************************************************************************
 * ListExpressionContext                                                       *
 *******************************************************************************/

/**
 * @class ListExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ListExpression_t {

    k_ASTNode_t* m_expressions;
};

typedef struct k_ListExpression_t k_ListExpression_t;

/*******************************************************************************
 * LabelClauseContext                                                          *
 *******************************************************************************/

// TODO: Use the ASTNode of the identifier instead of LabelClauseContext!

/**
 * @class LabelClauseContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LabelClause_t {

    k_Token_t* m_identifier;
};

/**
 * @memberof LabelClauseContext
 */
typedef struct k_LabelClause_t k_LabelClause_t;

/*******************************************************************************
 * IterativeStatementContext                                                   *
 *******************************************************************************/

/**
 * @class IterativeStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IterativeStatement_t {

    k_ASTNode_t* m_labelClause;
    k_ASTNode_t* m_statement;
};

/**
 * @memberof IterativeStatementContext
 */
typedef struct k_IterativeStatement_t k_IterativeStatement_t;

/*******************************************************************************
 * InclusiveOrExpressionContext                                                *
 *******************************************************************************/

/**
 * @class InclusiveOrExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_InclusiveOrExpression_t {

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
 * @memberof InclusiveOrExpressionContext
 */
typedef struct k_InclusiveOrExpression_t k_InclusiveOrExpression_t;

/*******************************************************************************
 * ImportDeclarationContext                                                    *
 *******************************************************************************/

/**
 * @class ImportDeclarationContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ImportDeclaration_t {

    jtk_ArrayList_t* m_identifiers; /* <k_ASTNode_t*> */
    bool m_wildcard;
};

/**
 * @memberof ImportDeclarationContext
 */
typedef struct k_ImportDeclaration_t k_ImportDeclaration_t;


/*******************************************************************************
 * IfStatementContext                                                          *
 *******************************************************************************/

/**
 * @class IfStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IfStatement_t {

    k_ASTNode_t* m_ifClause;
    jtk_ArrayList_t* m_elseIfClauses; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_elseClause;
};

/**
 * @memberof IfStatementContext
 */
typedef struct k_IfStatement_t k_IfStatement_t;


/*******************************************************************************
 * IfClauseContext                                                             *
 *******************************************************************************/

/**
 * @class IfClauseContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IfClause_t {

    k_ASTNode_t* m_expression;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof IfClauseContext
 */
typedef struct k_IfClause_t k_IfClause_t;

/*******************************************************************************
 * FunctionParametersContext                                                   *
 *******************************************************************************/

/**
 * @class FunctionParametersContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionParameters_t {

    jtk_ArrayList_t* m_fixedParameters; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_variableParameter;
};

/**
 * @memberof FunctionParametersContext
 */
typedef struct k_FunctionParameters_t k_FunctionParameters_t;

/*******************************************************************************
 * FunctionDeclarationContext                                                  *
 *******************************************************************************/

/**
 * @class FunctionDeclarationContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionDeclaration_t {

    k_Token_t* m_identifier;
    k_ASTNode_t* m_functionParameters;
    k_ASTNode_t* m_functionBody;
    k_Token_t* m_returnType;
    int32_t m_returnTypeDimensions;
};

/**
 * @memberof FunctionDeclarationContext
 */
typedef struct k_FunctionDeclaration_t k_FunctionDeclaration_t;

/*******************************************************************************
 * FunctionBodyContext                                                         *
 *******************************************************************************/

/**
 * @class FunctionBodyContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionBody_t {

    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof FunctionBodyContext
 */
typedef struct k_FunctionBody_t k_FunctionBody_t;

/*******************************************************************************
 * FunctionArgumentsContext                                                    *
 *******************************************************************************/

/**
 * @class FunctionArgumentsContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FunctionArguments_t {

    k_ASTNode_t* m_expressions;
};

/**
 * @memberof FunctionArgumentsContext
 */
typedef struct k_FunctionArguments_t k_FunctionArguments_t;

/*******************************************************************************
 * ForStatementContext                                                         *
 *******************************************************************************/

/**
 * @class ForStatementContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ForStatement_t {

    k_ASTNode_t* m_forParameter;
    k_ASTNode_t* m_expression;
    k_ASTNode_t* m_statementSuite;
    k_ASTNode_t* m_elseClause;
};

/**
 * @memberof ForStatementContext
 */
typedef struct k_ForStatement_t k_ForStatement_t;

/*******************************************************************************
 * ForParameterContext                                                        *
 *******************************************************************************/

/**
 * @class ForParameterContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ForParameter_t {

    bool m_declaration;
    bool m_variable;
    k_Token_t* m_identifier;
};

/**
 * @memberof ForParameterContext
 */
typedef struct k_ForParameter_t k_ForParameter_t;

/*******************************************************************************
 * FinallyClauseContext                                                        *
 *******************************************************************************/

/**
 * @class FinallyClauseContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FinallyClause_t {

    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof FinallyClauseContext
 */
typedef struct k_FinallyClause_t k_FinallyClause_t;

/*******************************************************************************
 * ExpressionsContext                                                          *
 *******************************************************************************/

/**
 * @class ExpressionsContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Expressions_t {

    jtk_ArrayList_t* m_expressions; /* <k_ASTNode_t*> */
};

/**
 * @memberof ExpressionsContext
 */
typedef struct k_Expressions_t k_Expressions_t;

/*******************************************************************************
 * ExpressionContext                                                           *
 *******************************************************************************/

/**
 * @class ExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Expression_t {

    k_ASTNode_t* m_assignmentExpression;
};

/**
 * @memberof ExpressionContext
 */
typedef struct k_Expression_t k_Expression_t;

/*******************************************************************************
 * ExclusiveOrExpressionContext                                                *
 *******************************************************************************/

/**
 * @class ExclusiveOrExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ExclusiveOrExpression_t {

    k_ASTNode_t* m_andExpression;
    jtk_ArrayList_t* m_andExpressions;
};

/**
 * @memberof ExclusiveOrExpressionContext
 */
typedef struct k_ExclusiveOrExpression_t k_ExclusiveOrExpression_t;

/*******************************************************************************
 * EqualityExpressionContext                                                   *
 *******************************************************************************/

/**
 * @class EqualityExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_EqualityExpression_t {

    k_ASTNode_t* m_relationalExpression;
    jtk_ArrayList_t* m_relationalExpressions;
};

/**
 * @memberof EqualityExpressionContext
 */
typedef struct k_EqualityExpression_t k_EqualityExpression_t;

/*******************************************************************************
 * EnumerationSuiteContext                                                     *
 *******************************************************************************/

/**
 * @class EnumerationSuiteContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_EnumerationSuite_t {

    jtk_ArrayList_t* m_enumerates; /* <k_ASTNode_t*> */
};

/**
 * @memberof EnumerationSuiteContext
 */
typedef struct k_EnumerationSuite_t k_EnumerationSuite_t;

/*******************************************************************************
 * ConditionalExpressionContext                                                *
 *******************************************************************************/

/**
 * @class ConditionalExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ConditionalExpression_t {

    k_ASTNode_t* m_logicalOrExpression;
    k_ASTNode_t* m_thenExpression;
    k_ASTNode_t* m_elseExpression;
};

/**
 * @memberof ConditionalExpressionContext
 */
typedef struct k_ConditionalExpression_t k_ConditionalExpression_t;

/*******************************************************************************
 * ClassSuiteContext                                                           *
 *******************************************************************************/

/**
 * @class ClassSuiteContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassSuite_t {

    jtk_ArrayList_t* m_classMembers; /* <k_ASTNode_t*> */
};

/**
 * @memberof ClassSuiteContext
 */
typedef struct k_ClassSuite_t k_ClassSuite_t;

/*******************************************************************************
 * ClassMemberContext                                                          *
 *******************************************************************************/

/**
 * @class ClassMemberContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassMember_t {

    k_ASTNode_t* m_annotations;
    jtk_ArrayList_t* m_modifiers; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_declaration;
};

/**
 * @memberof ClassMemberContext
 */
typedef struct k_ClassMember_t k_ClassMember_t;

/*******************************************************************************
 * ClassDeclarationContext                                                     *
 *******************************************************************************/

/**
 * @class ClassDeclarationContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ClassDeclaration_t {

    k_Token_t* m_identifier;
    k_ASTNode_t* m_structureSuite;
};

/**
 * @memberof ClassDeclarationContext
 */
typedef struct k_ClassDeclaration_t k_ClassDeclaration_t;

/*******************************************************************************
 * CatchFilterContext                                                          *
 *******************************************************************************/

/**
 * @class CatchFilterContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CatchFilter_t {

    jtk_ArrayList_t* m_typeNames; /* <k_ASTNode_t*> */
};

/**
 * @memberof CatchFilterContext
 */
typedef struct k_CatchFilter_t k_CatchFilter_t;

/*******************************************************************************
 * CatchClauseContext                                                          *
 *******************************************************************************/

/**
 * @class CatchClauseContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CatchClause_t {

    k_ASTNode_t* m_catchFilter;
    k_Token_t* m_identifier;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof CatchClauseContext
 */
typedef struct k_CatchClause_t k_CatchClause_t;

/*******************************************************************************
 * BreakStatementContext                                                       *
 *******************************************************************************/

/**
 * @class BreakStatementContext
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_BreakStatement_t {
    k_Token_t* m_identifier;
};

/**
 * @memberof BreakStatementContext
 */
typedef struct k_BreakStatement_t k_BreakStatement_t;

/*******************************************************************************
 * AssignmentExpressionContext                                                 *
 *******************************************************************************/

/**
 * @class AssignmentExpressionContext

 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_AssignmentExpression_t {
    k_ASTNode_t* m_conditionalExpression;
    k_ASTNode_t* m_assignmentOperator;
    k_ASTNode_t* m_assignmentExpression;
};

/**
 * @memberof AssignmentExpressionContext
 */
typedef struct k_AssignmentExpression_t k_AssignmentExpression_t;

#endif /* KUSH_PARSER_CONTEXT_H */