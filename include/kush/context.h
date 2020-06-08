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
#include <kush/scope.h>

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
#define TYPE_BOOLEAN 7

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
    Type boolean;
};

typedef struct Primitives Primitives;

/*******************************************************************************
 * ContextType                                                                 *
 *******************************************************************************/

enum ContextType {

    CONTEXT_UNKNOWN,

    CONTEXT_MODULE,

    CONTEXT_IMPORT_DECLARATION,

    CONTEXT_FUNCTION_DECLARATION,

    CONTEXT_BLOCK,

    CONTEXT_VARIABLE_DECLARATION,

    CONTEXT_BREAK_STATEMENT,

    CONTEXT_RETURN_STATEMENT,

    CONTEXT_THROW_STATEMENT,

    CONTEXT_IF_STATEMENT,

    CONTEXT_ITERATIVE_STATEMENT,

    CONTEXT_TRY_STATEMENT,

    CONTEXT_CATCH_CLAUSE,

    CONTEXT_STRUCTURE_DECLARATION,

    CONTEXT_ASSIGNMENT_EXPRESSION,

    CONTEXT_CONDITIONAL_EXPRESSION,

    CONTEXT_LOGICAL_OR_EXPRESSION,

    CONTEXT_LOGICAL_AND_EXPRESSION,

    CONTEXT_INCLUSIVE_OR_EXPRESSION,

    CONTEXT_EXCLUSIVE_OR_EXPRESSION,

    CONTEXT_AND_EXPRESSION,

    CONTEXT_EQUALITY_EXPRESSION,

    CONTEXT_RELATIONAL_EXPRESSION,

    CONTEXT_SHIFT_EXPRESSION,

    CONTEXT_ADDITIVE_EXPRESSION,

    CONTEXT_MULTIPLICATIVE_EXPRESSION,

    CONTEXT_UNARY_EXPRESSION,

    CONTEXT_POSTFIX_EXPRESSION,

    CONTEXT_SUBSCRIPT,

    CONTEXT_FUNCTION_ARGUMENTS,

    CONTEXT_MEMBER_ACCESS,

    CONTEXT_INITIALIZER_EXPRESSION,

    CONTEXT_ARRAY_EXPRESSION
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
    Scope* scope;
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
    BinaryExpression* condition;
    BinaryExpression* then;
    ConditionalExpression* otherwise;
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
    void* primary;
    bool token;
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
 * Subscript                                                                   *
 *******************************************************************************/

struct Subscript {
    ContextType tag;
    Token* bracket;
    BinaryExpression* expression;
};

typedef struct Subscript Subscript;

/*******************************************************************************
 * Block                                                              *
 *******************************************************************************/

struct Block {
    ContextType tag;
    jtk_ArrayList_t* statements;
    Scope* scope;
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
    jtk_ArrayList_t* parameters;
    FunctionParameter* variableParameter;
    Block* body;
    Token* returnType;
    int32_t returnTypeDimensions;
    Scope* scope;
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
    Scope* scope;
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

struct IterativeStatement {
    ContextType tag;
    Token* label;
    bool whileLoop;
    Token* parameter;
    BinaryExpression* expression;
    Block* body;
};

typedef struct IterativeStatement IterativeStatement;

/*******************************************************************************
 * TryStatement                                                                *
 *******************************************************************************/

struct TryStatement {
    ContextType tag;
    Block* tryClause;
    jtk_ArrayList_t* catchClauses;
    Block* finallyClause;
};

typedef struct TryStatement TryStatement;

/*******************************************************************************
 * CatchClause                                                                 *
 *******************************************************************************/

/**
 * Not part of the AST.
 */
struct CatchClause {
    jtk_ArrayList_t* captures;
    Token* parameter;
    Block* body;
};

typedef struct CatchClause CatchClause;

/*******************************************************************************
 * Variable                                                                    *
 *******************************************************************************/

/**
 * Not part of the AST.
 */
struct Variable {
    bool infer;
    bool constant;
    Token* baseType;
    int32_t dimensions;
    Token* identifier;
    BinaryExpression* expression;
    Scope* parent;
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

struct ReturnStatement {
    ContextType tag;
    BinaryExpression* expression;
};

typedef struct ReturnStatement ReturnStatement;

/*******************************************************************************
 * BreakStatement                                                              *
 *******************************************************************************/

struct BreakStatement {
    ContextType tag;
    Token* identifier;
};

typedef struct BreakStatement BreakStatement;

#endif /* KUSH_PARSER_CONTEXT_H */