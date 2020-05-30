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

#ifndef KUSH_COMPILER_COMPILER_AST_NODE_H
#define KUSH_COMPILER_COMPILER_AST_NODE_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNodeType.h>

/******************************************************************************
 * ContextDestructorFunction                                                  *
 ******************************************************************************/

typedef void (*k_ContextDestructorFunction_t)(void* context);

/*******************************************************************************
 * EnumerateContextChildrenFunction                                            *
 ******************************************************************************/

typedef void (*k_EnumerateContextChildrenFunction_t)(void* context, jtk_ArrayList_t* children);

/******************************************************************************
 * ASTNode																	  *
 ******************************************************************************/

/**
 * Nodes are populated in the parse tree at various depths.
 * A node may be a terminal or a rule. This is identified by
 * the node's type. Moreover, type specific data is abstracted
 * away as payload. For terminals, the payload may be cast to
 * k_Token_t*. For rules, the payload may be cast to
 * their specific contexts, such as k_ReturnStatementContext_t*.
 *
 * Every node, except the root node, has a parent. The root nodes
 * parent is always NULL. Parent nodes are guaranteed to be rules.
 * Because a terminal always represents a leaf.
 *
 * @class ASTNode
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_ASTNode_t {
    k_ASTNodeType_t m_type;
    void* m_context;
    struct k_ASTNode_t* m_parent;
    jtk_ArrayList_t* m_children;
    k_ContextDestructorFunction_t m_contextDestructor;
    k_EnumerateContextChildrenFunction_t m_enumerateContextChildren;
};

/**
 * @memberof ASTNode
 */
typedef struct k_ASTNode_t k_ASTNode_t;

// Constructor

/**
 * @memberof ASTNode
 */
k_ASTNode_t* k_ASTNode_new(k_ASTNode_t* parent);

// Destructor

/**
 * @memberof ASTNode
 */
void k_ASTNode_delete(k_ASTNode_t* node);

// Children

/**
 * @memberof ASTNode
 */
jtk_ArrayList_t* k_ASTNode_getChildren(k_ASTNode_t* node);

// Context

/**
 * @memberof ASTNode
 */
void* k_ASTNode_getContext(k_ASTNode_t* node);

// Depth

/**
 * @memberof ASTNode
 */
int32_t k_ASTNode_getDepth(k_ASTNode_t* node);

// Error

/**
 * @memberof ASTNode
 */
bool k_ASTNode_isErroneous(k_ASTNode_t* node);

// Parent

/**
 * @memberof ASTNode
 */
k_ASTNode_t* k_ASTNode_getParent(k_ASTNode_t* node);

// Rule

/**
 * @memberof ASTNode
 */
bool k_ASTNode_isRule(k_ASTNode_t* node);

// Terminal

/**
 * @memberof ASTNode
 */
bool k_ASTNode_isTerminal(k_ASTNode_t* node);

// Type

/**
 * @memberof ASTNode
 */
k_ASTNodeType_t k_ASTNode_getType(k_ASTNode_t* node);

// String

uint8_t* k_ASTNode_toCString(k_ASTNode_t* node, int32_t* size);

#endif /* KUSH_COMPILER_COMPILER_AST_NODE_H */
