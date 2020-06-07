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
#include <kush/token.h>

/*******************************************************************************
 * Type                                                                        *
 *******************************************************************************/

#define TYPE_STRUCTURE 0
#define TYPE_INTEGER 1
#define TYPE_DECIMAL 2
#define TYPE_ARRAY 3
#define TYPE_VOID 4
#define TYPE_NULL 5
#define TYPE_STRING 6

typedef struct Type Type;
typedef struct Structure Structure;

struct Type {
    uint8_t tag;
    bool indexable;
    bool accessible;
    bool callable;
    union {
        struct {
            Type* base;
            uint16_t dimensions;
        } array;
        struct {
            uint8_t size;
        } integer;
        struct {
            uint8_t size;
        } decimal;
        Structure* structure;
    };
};

/*******************************************************************************
 * Primitives                                                                  *
 *******************************************************************************/

struct Primitives {
    Type i8;
    Type i16;
    Type i32;
    Type i64;
    Type f32;
    Type f64;
    Type void_;
    Type null;
    Type string;
};

typedef struct Primitives Primitives;

/*******************************************************************************
 * ContextType                                                                 *
 *******************************************************************************/

enum ContextType {

    K_CONTEXT_UNKNOWN,

    K_CONTEXT_COMPILATION_UNIT,

    K_CONTEXT_IMPORT_DECLARATION,

    K_CONTEXT_FUNCTION_DECLARATION,

    K_CONTEXT_FUNCTION_PARAMETER,

    K_CONTEXT_BLOCK_STATEMENT,

    K_CONTEXT_VARIABLE_DECLARATION,

    K_CONTEXT_VARIABLE_DECLARATOR,

    K_CONTEXT_BREAK_STATEMENT,

    K_CONTEXT_RETURN_STATEMENT,

    K_CONTEXT_THROW_STATEMENT,

    K_CONTEXT_IF_STATEMENT,

    K_CONTEXT_IF_CLAUSE,

    K_CONTEXT_ITERATIVE_STATEMENT,

    K_CONTEXT_TRY_STATEMENT,

    K_CONTEXT_CATCH_CLAUSE,

    K_CONTEXT_STRUCTURE_DECLARATION,

    K_CONTEXT_ASSIGNMENT_EXPRESSION,

    K_CONTEXT_CONDITIONAL_EXPRESSION,

    K_CONTEXT_LOGICAL_OR_EXPRESSION,

    K_CONTEXT_LOGICAL_AND_EXPRESSION,

    K_CONTEXT_INCLUSIVE_OR_EXPRESSION,

    K_CONTEXT_EXCLUSIVE_OR_EXPRESSION,

    K_CONTEXT_AND_EXPRESSION,

    K_CONTEXT_EQUALITY_EXPRESSION,

    K_CONTEXT_RELATIONAL_EXPRESSION,

    K_CONTEXT_SHIFT_EXPRESSION,

    K_CONTEXT_ADDITIVE_EXPRESSION,

    K_CONTEXT_MULTIPLICATIVE_EXPRESSION,

    K_CONTEXT_UNARY_EXPRESSION,

    K_CONTEXT_POSTFIX_EXPRESSION,

    K_CONTEXT_INITIALIZER_EXPRESSION,

    K_CONTEXT_ARRAY_EXPRESSION
};

typedef enum ContextType ContextType;

/*******************************************************************************
 * Context                                                                     *
 *******************************************************************************/

struct Context {
    ContextType tag;
};

typedef struct Context Context;

/*******************************************************************************
 * Module                                                                      *
 *******************************************************************************/

struct Module {
    ContextType tag;
	jtk_ArrayList_t* imports;
	jtk_ArrayList_t* functions;
    jtk_ArrayList_t* structures;
};

typedef struct Module Module;

/*******************************************************************************
 * ImportDeclaration                                                           *
 *******************************************************************************/

struct ImportDeclaration {
    ContextType tag;
    bool wildcard;
    jtk_ArrayList_t* identifiers;
};

typedef struct ImportDeclaration ImportDeclaration;

/*******************************************************************************
 * BinaryExpression                                                            *
 *******************************************************************************/

typedef struct BinaryExpression BinaryExpression;

struct BinaryExpression {
    ContextType tag;
    BinaryExpression* left;
    jtk_ArrayList_t* others;
};

/*******************************************************************************
 * ConditionalExpression                                                       *
 *******************************************************************************/

typedef struct ConditionalExpression ConditionalExpression;

struct ConditionalExpression {
    ContextType tag;
    BinaryExpression* logicalOrExpression;
    BinaryExpression* thenExpression;
    ConditionalExpression* elseExpression;
};

/*******************************************************************************
 * UnaryExpression                                                             *
 *******************************************************************************/

struct UnaryExpression {
    ContextType tag;
    Token* operator;
    Context* expression; // Unary expression or postfix expression
};

typedef struct UnaryExpression UnaryExpression;

/*******************************************************************************
 * PostfixExpression                                                           *
 *******************************************************************************/

struct PostfixExpression {
    ContextType tag;
    void* primaryExpression;
    bool primaryToken;
    jtk_ArrayList_t* postfixParts; // contexts
};

typedef struct PostfixExpression PostfixExpression;

/*******************************************************************************
 * MemberAccess                                                                *
 *******************************************************************************/

struct MemberAccess {
    ContextType tag;
    Token* identifier;
};

typedef struct MemberAccess MemberAccess;

/*******************************************************************************
 * InitializerExpression                                                               *
 *******************************************************************************/

struct InitializerExpression {
    ContextType tag;
    jtk_ArrayList_t* entries; // Pair<Token, BinaryExpression>
};

typedef struct InitializerExpression InitializerExpression;

/*******************************************************************************
 * ArrayExpression                                                              *
 *******************************************************************************/

struct ArrayExpression {
    ContextType tag;
    jtk_ArrayList_t* expressions;
};

typedef struct ArrayExpression ArrayExpression;

/*******************************************************************************
 * FunctionArguments                                                           *
 *******************************************************************************/

struct FunctionArguments {
    ContextType tag;
    Token* parenthesis;
    jtk_ArrayList_t* expressions;
};

typedef struct FunctionArguments FunctionArguments;

/*******************************************************************************
 * Block                                                              *
 *******************************************************************************/

struct Block {
    ContextType tag;
    jtk_ArrayList_t* statements;
};

typedef struct Block Block;

/*******************************************************************************
 * FunctionParameter                                                           *
 *******************************************************************************/

struct FunctionParameter {
    ContextType tag;
    Token* baseType;
    int32_t dimensions;
    Token* identifier;
};

typedef struct FunctionParameter FunctionParameter;

/*******************************************************************************
 * Function                                                                    *
 *******************************************************************************/

struct Function {
    ContextType tag;
    Token* identifier;
    jtk_ArrayList_t* fixedParameters;
    FunctionParameter* variableParameter;
    Block* body;
    Token* returnType;
    int32_t returnTypeDimensions;
};

typedef struct Function Function;

/*******************************************************************************
 * Structure                                                            *
 *******************************************************************************/

struct Structure {
    ContextType tag;
    Token* identifier;
    jtk_ArrayList_t* variables;
    Type* type;
};

typedef struct Structure Structure;

/*******************************************************************************
 * IfClause                                                                    *
 *******************************************************************************/

struct IfClause {
    ContextType tag;
    BinaryExpression* expression;
    Block* body;
};

typedef struct IfClause IfClause;

/*******************************************************************************
 * IfStatement                                                                 *
 *******************************************************************************/

struct IfStatement {
    ContextType tag;
    IfClause* ifClause;
    jtk_ArrayList_t* elseIfClauses;
    Block* elseClause;
};

typedef struct IfStatement IfStatement;

/*******************************************************************************
 * IterativeStatement                                                          *
 *******************************************************************************/

struct k_IterativeStatement_t {
    ContextType tag;
    Token* label;
    bool whileLoop;
    Token* parameter;
    BinaryExpression* expression;
    Block* blockStatement;
};

typedef struct k_IterativeStatement_t k_IterativeStatement_t;

/*******************************************************************************
 * TryStatement                                                                *
 *******************************************************************************/

struct k_TryStatement_t {
    ContextType tag;
    Block* tryClause;
    jtk_ArrayList_t* catchClauses;
    Block* finallyClause;
};

typedef struct k_TryStatement_t k_TryStatement_t;

/*******************************************************************************
 * CatchClause                                                                 *
 *******************************************************************************/

struct CatchClause {
    jtk_ArrayList_t* captures;
    Token* parameter;
    Block* body;
};

typedef struct CatchClause CatchClause;

/*******************************************************************************
 * Variable                                                                    *
 *******************************************************************************/

struct Variable {
    ContextType tag;
    bool infer;
    bool constant;
    Token* baseType;
    int32_t dimensions;
    Token* identifier;
    BinaryExpression* expression;
};

typedef struct Variable Variable;

/*******************************************************************************
 * VariableDeclaration                                                         *
 *******************************************************************************/

struct VariableDeclaration {
    ContextType tag;
    jtk_ArrayList_t* variables;
};

typedef struct VariableDeclaration VariableDeclaration;

/*******************************************************************************
 * ThrowStatement                                                              *
 *******************************************************************************/

struct ThrowStatement {
    ContextType tag;
    BinaryExpression* expression;
};

typedef struct ThrowStatement ThrowStatement;

/*******************************************************************************
 * ReturnStatement                                                             *
 *******************************************************************************/

struct k_ReturnStatement_t {
    ContextType tag;
    BinaryExpression* expression;
};

typedef struct k_ReturnStatement_t k_ReturnStatement_t;

/*******************************************************************************
 * BreakStatement                                                              *
 *******************************************************************************/

struct k_BreakStatement_t {
    ContextType tag;
    Token* identifier;
};

typedef struct k_BreakStatement_t k_BreakStatement_t;

#endif /* KUSH_PARSER_CONTEXT_H */