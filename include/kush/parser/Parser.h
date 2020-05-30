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

// Sunday, November 05, 2017

#ifndef KUSH_COMPILER_PARSER_PARSER_H
#define KUSH_COMPILER_PARSER_PARSER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>
#include <kush/lexer/TokenStream.h>

/*******************************************************************************
 * Parser                                                                      *
 *******************************************************************************/

/**
 * @class Parser
 * @ingroup k_compiler_parser
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Parser_t {
    k_Compiler_t* m_compiler;
    k_TokenStream_t* m_tokens;
    k_TokenType_t* m_followSet;
    int32_t m_followSetSize;
    int32_t m_followSetCapacity;
    bool m_recovery;
    k_ASTNodeType_t m_mainComponent;
};

/**
 * @memberof Parser
 */
typedef struct k_Parser_t k_Parser_t;

// Constructor

/**
 * @memberof Parser
 */
k_Parser_t* k_Parser_new(k_Compiler_t* compiler, k_TokenStream_t* tokens);

// Destructor

/**
 * @memberof Parser
 */
void k_Parser_delete(k_Parser_t* parser);

// Rule

/**
 * @memberof Parser
 */
const char* k_Parser_getRuleName(k_ASTNodeType_t type);

// Parse

/**
 * @memberof Parser
 */
void k_Parser_parse(k_Parser_t* parser);

// Match

/**
 * @memberof Parser
 */
void k_Parser_match(k_Parser_t* parser, k_TokenType_t type);

/**
 * @memberof Parser
 */
k_Token_t* k_Parser_matchAndYield(k_Parser_t* parser, k_TokenType_t type);

// Recovery

void k_Parser_pushFollowToken(k_Parser_t* parser, k_TokenType_t type);
void k_Parser_popFollowToken(k_Parser_t* parser);
void k_Parser_recover(k_Parser_t* parser);

// Reset

/**
 * @memberof Parser
 */
void k_Parser_reset(k_Parser_t* parser, k_TokenStream_t* tokens);

// Rules

/**
 * @memberof Parser
 */
void k_Parser_compilationUnit(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_importDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isAnnotatedComponentDeclarationFollow(k_TokenType_t token);

/**
 * @memberof Parser
 */
void k_Parser_annotatedComponentDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_annotations(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_annotation(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_annotationType(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_annotationAttribute(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_componentDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_functionDeclaration(k_Parser_t* parser, k_ASTNode_t* node, uint32_t modifiers);

/**
 * @memberof Parser
 */
void k_Parser_functionParameters(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_functionBody(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_statementSuite(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_simpleStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isSimpleStatementFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_statement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isStatementFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_emptyStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_variableDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_variableDeclarator(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_constantDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_constantDeclarator(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_assertStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_breakStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_continueStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_returnStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_throwStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_compoundStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isCompoundStatementFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_ifStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_ifClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_elseIfClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_elseClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_iterativeStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_labelClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_whileStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_forStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_tryStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_tryClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_catchClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_catchFilter(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_typeName(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_finallyClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_synchronizeStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_withStatement(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_classDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_classExtendsClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_classSuite(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isClassMemberFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_classMember(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isClassMemberModifier(k_TokenType_t type);

/**
 * @memberof Parser
 */
// void k_Parser_constructorDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_enumerationDeclaration(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_enumerationBaseClause(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_enumerationSuite(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_enumerate(k_Parser_t* paresr, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_expressions(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_expression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isExpressionFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_assignmentExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isAssignmentOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_conditionalExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_logicalOrExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_logicalAndExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_inclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_exclusiveOrExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_andExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_equalityExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isEqualityOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_relationalExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isRelationalOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_shiftExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isShiftOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_additiveExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isAdditiveOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_multiplicativeExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isMultiplicativeOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_unaryExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isUnaryExpressionFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
bool k_Parser_isUnaryOperator(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_postfixExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isPostfixExpressionFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
bool k_Parser_isPostfixPartFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_subscript(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_functionArguments(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_memberAccess(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_postfixOperator(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_primaryExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isPrimaryExpressionFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_identifier(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_literal(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
bool k_Parser_isLiteral(k_TokenType_t type);

/**
 * @memberof Parser
 */
bool k_Parser_isLiteralFollow(k_TokenType_t type);

/**
 * @memberof Parser
 */
void k_Parser_mapExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_mapEntries(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_mapEntry(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_listExpression(k_Parser_t* parser, k_ASTNode_t* node);

/**
 * @memberof Parser
 */
void k_Parser_newExpression(k_Parser_t* parser, k_ASTNode_t* node);

#endif /* KUSH_COMPILER_PARSER_PARSER_H */
