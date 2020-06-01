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
#include <kush/parser/ContextType.h>
#include <kush/lexer/Token.h>

/*******************************************************************************
 * Context                                                                     *
 *******************************************************************************/

struct k_Context_t {
    k_ContextType_t m_type;
};

typedef struct k_Context_t k_Context_t;

/*******************************************************************************
 * CompilationUnit                                                             *
 *******************************************************************************/

/**
 * @class CompilationUnit
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_CompilationUnit_t {
    k_ContextType_t m_type;
	jtk_ArrayList_t* m_imports;
	jtk_ArrayList_t* m_functions;
    jtk_ArrayList_t* m_structures;
};

/**
 * @memberof CompilationUnit
 */
typedef struct k_CompilationUnit_t k_CompilationUnit_t;

/*******************************************************************************
 * BinaryExpression                                                            *
 *******************************************************************************/

/**
 * @memberof BinaryExpression
 */
typedef struct k_BinaryExpression_t k_BinaryExpression_t;

/**
 * @class BinaryExpression
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_BinaryExpression_t {
    k_ContextType_t m_type;
    k_BinaryExpression_t* m_left;
    jtk_ArrayList_t* m_others;
};

/*******************************************************************************
 * VariableDeclaration                                                         *
 *******************************************************************************/

/**
 * @class VariableDeclaration
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_VariableDeclaration_t {
    k_ContextType_t m_type;
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
 * @since kush 0.1
 */
struct k_VariableDeclarator_t {
    k_ContextType_t m_type;
    k_Token_t* m_identifier;
    k_BinaryExpression_t* m_expression;
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
 * @since kush 0.1
 */
struct k_TryStatement_t {
    k_ContextType_t m_type;
    k_BlockStatement_t* m_tryClause;
    jtk_ArrayList_t* m_catchClauses;
    k_BlockStatement_t* m_finallyClause;
};

/**
 * @memberof TryStatement
 */
typedef struct k_TryStatement_t k_TryStatement_t;

/*******************************************************************************
 * ThrowStatement                                                              *
 *******************************************************************************/

/**
 * @class ThrowStatement
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_ThrowStatement_t {
    k_ContextType_t m_type;
    k_BinaryExpression_t* m_expression;
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
 * @since kush 0.1
 */
struct k_ReturnStatement_t {
    k_ContextType_t m_type;
    k_BinaryExpression_t* m_expression;
};

/**
 * @memberof ReturnStatement
 */
typedef struct k_ReturnStatement_t k_ReturnStatement_t;

/*******************************************************************************
 * BlockStatement                                                              *
 *******************************************************************************/

/**
 * @class BlockStatement
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_BlockStatement_t {
    k_ContextType_t m_type;
    jtk_ArrayList_t* m_statements;
};

/**
 * @memberof BlockStatement
 */
typedef struct k_BlockStatement_t k_BlockStatement_t;

/*******************************************************************************
 * UnaryExpression                                                             *
 *******************************************************************************/

/**
 * @class UnaryExpression

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_UnaryExpression_t {
    k_ContextType_t m_type;
    k_Token_t* m_operator;
    k_Context_t* m_expression; // Unary expression or postfix expression
};

/**
 * @memberof UnaryExpression
 */
typedef struct k_UnaryExpression_t k_UnaryExpression_t;

/*******************************************************************************
 * PostfixExpression                                                           *
 *******************************************************************************/

/**
 * @class PostfixExpression

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_PostfixExpression_t {
    k_ContextType_t m_type;
    void* m_primaryExpression;
    bool m_primaryToken;
    jtk_ArrayList_t* m_postfixParts; // contexts
};

/**
 * @memberof PostfixExpression
 */
typedef struct k_PostfixExpression_t k_PostfixExpression_t;

/*******************************************************************************
 * MemberAccess                                                                *
 *******************************************************************************/

/**
 * @class MemberAccess

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_MemberAccess_t {
    k_ContextType_t m_type;
    k_Token_t* m_identifier;
};

/**
 * @memberof MemberAccess
 */
typedef struct k_MemberAccess_t k_MemberAccess_t;

/*******************************************************************************
 * InitializerExpression                                                               *
 *******************************************************************************/

/**
 * @class InitializerExpression

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_InitializerExpression_t {
    k_ContextType_t m_type;
    jtk_ArrayList_t* m_entries; // Pair<Token, BinaryExpression>
};

typedef struct k_InitializerExpression_t k_InitializerExpression_t;

/*******************************************************************************
 * ArrayExpression                                                              *
 *******************************************************************************/

/**
 * @class ArrayExpression
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_ArrayExpression_t {
    k_ContextType_t m_type;
    jtk_ArrayList_t* m_expressions;
};

typedef struct k_ArrayExpression_t k_ArrayExpression_t;

/*******************************************************************************
 * IterativeStatement                                                          *
 *******************************************************************************/

/**
 * @class IterativeStatement
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_IterativeStatement_t {
    k_ContextType_t m_type;
    k_Token_t* m_label;
    bool m_while;
    k_Token_t* m_parameter;
    k_BinaryExpression_t* m_expression;
    k_BlockStatement_t* m_blockStatement;
};

/**
 * @memberof IterativeStatement
 */
typedef struct k_IterativeStatement_t k_IterativeStatement_t;

/*******************************************************************************
 * ImportDeclaration                                                           *
 *******************************************************************************/

/**
 * @class ImportDeclaration

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_ImportDeclaration_t {
    k_ContextType_t m_type;
    bool m_wildcard;
    jtk_ArrayList_t* m_identifiers;
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
 * @since kush 0.1
 */
struct k_IfStatement_t {
    k_ContextType_t m_type;
    k_IfClause_t* m_ifClause;
    jtk_ArrayList_t* m_elseIfClauses;
    k_BlockStatement_t* m_elseClause;
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
 * @since kush 0.1
 */
struct k_IfClause_t {
    k_ContextType_t m_type;
    k_BinaryExpression_t* m_expression;
    k_BlockStatement_t* m_body;
};

/**
 * @memberof IfClause
 */
typedef struct k_IfClause_t k_IfClause_t;

/*******************************************************************************
 * FunctionDeclaration                                                         *
 *******************************************************************************/

/**
 * @class FunctionDeclaration
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_FunctionDeclaration_t {
    k_ContextType_t m_type;
    k_Token_t* m_identifier;
    jtk_ArrayList_t* m_fixedParameters;
    k_FunctionParameter_t* m_variableParameter;
    k_BlockStatement_t* m_body;
    k_Token_t* m_returnType;
    int32_t m_returnTypeDimensions;
};

/**
 * @memberof FunctionDeclaration
 */
typedef struct k_FunctionDeclaration_t k_FunctionDeclaration_t;

/*******************************************************************************
 * FunctionArguments                                                           *
 *******************************************************************************/

/**
 * @class FunctionArguments
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_FunctionArguments_t {
    k_ContextType_t m_type;
    jtk_ArrayList_t* m_expressions;
};

/**
 * @memberof FunctionArguments
 */
typedef struct k_FunctionArguments_t k_FunctionArguments_t;

/*******************************************************************************
 * ConditionalExpression                                                       *
 *******************************************************************************/

/**
 * @memberof ConditionalExpression
 */
typedef struct k_ConditionalExpression_t k_ConditionalExpression_t;

/**
 * @class ConditionalExpression
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_ConditionalExpression_t {
    k_ContextType_t m_type;
    k_BinaryExpression_t* m_logicalOrExpression;
    k_BinaryExpression_t* m_thenExpression;
    k_ConditionalExpression_t* m_elseExpression;
};

/*******************************************************************************
 * StructureDeclaration                                                            *
 *******************************************************************************/

/**
 * @class StructureDeclaration

 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_StructureDeclaration_t {
    k_ContextType_t m_type;
    k_Token_t* m_identifier;
    jtk_ArrayList_t* m_variables;
};

/**
 * @memberof StructureDeclaration
 */
typedef struct k_StructureDeclaration_t k_StructureDeclaration_t;


/*******************************************************************************
 * CatchClause                                                                 *
 *******************************************************************************/

struct k_CatchClause_t {
    jtk_ArrayList_t* m_captures;
    k_Token_t* m_parameter;
    k_BlockStatement_t* m_body;
};

typedef struct k_CatchClause_t k_CatchClause_t;

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
 * @since kush 0.1
 */
struct k_BreakStatement_t {
    k_ContextType_t m_type;
    k_Token_t* m_identifier;
};

/**
 * @memberof BreakStatement
 */
typedef struct k_BreakStatement_t k_BreakStatement_t;

/*******************************************************************************
 * FunctionParameter                                                           *
 *******************************************************************************/

struct k_FunctionParameter_t {
    k_ContextType_t m_type;
    k_Token_t* m_baseType;
    int32_t m_dimensions;
    k_Token_t* m_identifier;
};

typedef struct k_FunctionParameter_t k_FunctionParameter_t;

/*******************************************************************************
 * StorageDeclarator                                                           *
 *******************************************************************************/

struct k_StorageDeclarator_t {
    k_ContextType_t m_type;
    bool m_infer;
    bool m_constant;
    k_Token_t* m_baseType;
    int32_t m_dimensions;
    k_Token_t* m_identifier;
    k_BinaryExpression_t* m_expression;
};

typedef struct k_StorageDeclarator_t k_StorageDeclarator_t;

#endif /* KUSH_PARSER_CONTEXT_H */