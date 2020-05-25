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

// Saturday, April 28, 2018

#ifndef COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_AST_HELPER_H
#define COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_AST_HELPER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>
#include <kush/lexer/TokenType.h>

/*******************************************************************************
 * ASTHelper                                                                   *
 *******************************************************************************/

// Ancestor

/**
 * @memberof ASTHelper
 */
bool k_ASTHelper_isAncestor(k_ASTNode_t* node, k_ASTNode_t* subject);

/**
 * @memberof ASTHelper
 */
k_ASTNode_t* k_ASTHelper_getAncestor(k_ASTNode_t* node, k_ASTNodeType_t type);

// Descendant

/**
 * @memberof ASTHelper
 */
bool k_ASTHelper_isDescendant(k_ASTNode_t* node, k_ASTNode_t* subject);

// Node

/**
 * @memberof ASTHelper
 */
void k_ASTHelper_getNodes(k_ASTNode_t* node, jtk_ArrayList_t* list, int32_t filter, bool captureTerminals, bool strip);

// Tokens

/**
 * @memberof ASTHelper
 */
void k_ASTHelper_getTokens(k_ASTNode_t* node, jtk_ArrayList_t* list);

/**
 * @memberof ASTHelper
 */
void k_ASTHelper_getFilteredTokens(k_ASTNode_t* node, jtk_ArrayList_t* list, k_TokenType_t type);

// Terminal Node

/**
 * @memberof ASTHelper
 */
void k_ASTHelper_getTerminalNodes(k_ASTNode_t* node, jtk_ArrayList_t* list);

/**
 * @memberof ASTHelper
 */
void k_ASTHelper_getFilteredTerminalNodes(k_ASTNode_t* node, jtk_ArrayList_t* list, k_TokenType_t type);

#endif /* COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_AST_HELPER_H */