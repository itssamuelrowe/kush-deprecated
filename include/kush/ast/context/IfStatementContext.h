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

// Wednesday, July 17, 2019

#ifndef KUSH_COMPILER_AST_CONTEXT_IF_STATEMENT_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_IF_STATEMENT_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * IfStatementContext                                                          *
 *******************************************************************************/

/**
 * @class IfStatementContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_IfStatementContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_ifClause;
    jtk_ArrayList_t* m_elseIfClauses; /* <k_ASTNode_t*> */
    k_ASTNode_t* m_elseClause;
};

/**
 * @memberof IfStatementContext
 */
typedef struct k_IfStatementContext_t k_IfStatementContext_t;

// Constructor

/**
 * @memberof IfStatementContext
 */
k_IfStatementContext_t* k_IfStatementContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof IfStatementContext
 */
void k_IfStatementContext_delete(k_IfStatementContext_t* context);

// Children

/**
 * @memberof IfStatementContext
 */
void k_IfStatementContext_getChildren(k_IfStatementContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_IF_STATEMENT_CONTEXT_H */
