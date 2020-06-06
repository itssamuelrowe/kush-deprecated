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
    k_ContextType_t tag;
};

typedef struct k_Context_t k_Context_t;

/*******************************************************************************
 * Module                                                                      *
 *******************************************************************************/

/**
 * @class Module
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_Module_t {
    k_ContextType_t tag;
	jtk_ArrayList_t* imports;
	jtk_ArrayList_t* functions;
    jtk_ArrayList_t* structures;
};

/**
 * @memberof Module
 */
typedef struct k_Module_t k_Module_t;

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
    k_ContextType_t tag;
    k_BinaryExpression_t* left;
    jtk_ArrayList_t* others;
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
    k_ContextType_t tag;
    jtk_ArrayList_t* variables;
};

/**
 * @memberof VariableDeclaration
 */
typedef struct k_VariableDeclaration_t k_VariableDeclaration_t;

/*******************************************************************************
 * TryStatement                                                                *
 *******************************************************************************/

/**
 * @class TryStatement
 * @author Samuel Rowe
 * @since kush 0.1
 */
struct k_TryStatement_t {
    k_ContextType_t tag;
    k_BlockStatement_t* tryClause;
    jtk_ArrayList_t* catchClauses;
    k_BlockStatement_t* finallyClause;
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
    k_ContextType_t tag;
    k_BinaryExpression_t* expression;
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
    k_ContextType_t tag;
    k_BinaryExpression_t* expression;
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
    k_ContextType_t tag;
    jtk_ArrayList_t* statements;
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
    k_ContextType_t tag;
    k_Token_t* operator;
    k_Context_t* expression; // Unary expression or postfix expression
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
    k_ContextType_t tag;
    void* primaryExpression;
    bool primaryToken;
    jtk_ArrayList_t* postfixParts; // contexts
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
    k_ContextType_t tag;
    k_Token_t* identifier;
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
    k_ContextType_t tag;
    jtk_ArrayList_t* entries; // Pair<Token, BinaryExpression>
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
    k_ContextType_t tag;
    jtk_ArrayList_t* expressions;
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
    k_ContextType_t tag;
    k_Token_t* label;
    bool while;
    k_Token_t* parameter;
    k_BinaryExpression_t* expression;
    k_BlockStatement_t* blockStatement;
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
    k_ContextType_t tag;
    bool wildcard;
    jtk_ArrayList_t* identifiers;
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
    k_ContextType_t tag;
    k_IfClause_t* ifClause;
    jtk_ArrayList_t* elseIfClauses;
    k_BlockStatement_t* elseClause;
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
    k_ContextType_t tag;
    k_BinaryExpression_t* expression;
    k_BlockStatement_t* body;
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
    k_ContextType_t tag;
    k_Token_t* identifier;
    jtk_ArrayList_t* fixedParameters;
    k_FunctionParameter_t* variableParameter;
    k_BlockStatement_t* body;
    k_Token_t* returnType;
    int32_t returnTypeDimensions;
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
    k_ContextType_t tag;
    k_Token_t* parenthesis;
    jtk_ArrayList_t* expressions;
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
    k_ContextType_t tag;
    k_BinaryExpression_t* logicalOrExpression;
    k_BinaryExpression_t* thenExpression;
    k_ConditionalExpression_t* elseExpression;
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
    k_ContextType_t tag;
    k_Token_t* identifier;
    jtk_ArrayList_t* variables;
    k_Type_t* type;
};

/**
 * @memberof StructureDeclaration
 */
typedef struct k_StructureDeclaration_t k_StructureDeclaration_t;


/*******************************************************************************
 * CatchClause                                                                 *
 *******************************************************************************/

struct k_CatchClause_t {
    jtk_ArrayList_t* captures;
    k_Token_t* parameter;
    k_BlockStatement_t* body;
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
    k_ContextType_t tag;
    k_Token_t* identifier;
};

/**
 * @memberof BreakStatement
 */
typedef struct k_BreakStatement_t k_BreakStatement_t;

/*******************************************************************************
 * FunctionParameter                                                           *
 *******************************************************************************/

struct k_FunctionParameter_t {
    k_ContextType_t tag;
    k_Token_t* baseType;
    int32_t dimensions;
    k_Token_t* identifier;
};

typedef struct k_FunctionParameter_t k_FunctionParameter_t;

/*******************************************************************************
 * StorageDeclarator                                                           *
 *******************************************************************************/

struct k_StorageDeclarator_t {
    k_ContextType_t tag;
    bool infer;
    bool constant;
    k_Token_t* baseType;
    int32_t dimensions;
    k_Token_t* identifier;
    k_BinaryExpression_t* expression;
};

typedef struct k_StorageDeclarator_t k_StorageDeclarator_t;

/*******************************************************************************
 * Type                                                                        *
 *******************************************************************************/

#define K_TYPE_STRUCTURE 0
#define K_TYPE_INTEGER 1
#define K_TYPE_DECIMAL 2
#define K_TYPE_ARRAY 3
#define K_TYPE_VOID 4
#define K_TYPE_NULL 5
#define K_TYPE_STRING 6

struct k_Type_t {
    uint8_t tag;
    bool indexable;
    bool accessible;
    bool callable;
    union {
        struct {
            k_Type_t* m_base;
            uint16_t m_dimensions;
        } array;
        struct {
            uint8_t size;
        } integer;
        struct {
            uint8_t size;
        } decimal;
        k_StructureDeclaration_t* structure;
    };
};

typedef struct k_Type_t k_Type_t;

struct k_Primitives_t {
    k_Type_t i8;
    k_Type_t i16;
    k_Type_t i32;
    k_Type_t i64;
    k_Type_t f32;
    k_Type_t f64;
    k_Type_t void_;
    k_Type_t null;
    k_Type_t string;
};

typedef struct k_Primitives_t k_Primitives_t;

#endif /* KUSH_PARSER_CONTEXT_H */