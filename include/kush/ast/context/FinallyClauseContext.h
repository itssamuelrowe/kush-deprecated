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

#ifndef KUSH_COMPILER_AST_CONTEXT_FINALLY_CLAUSE_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_FINALLY_CLAUSE_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * FinallyClauseContext                                                        *
 *******************************************************************************/

/**
 * @class FinallyClauseContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_FinallyClauseContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_statementSuite;
};

/**
 * @memberof FinallyClauseContext
 */
typedef struct k_FinallyClauseContext_t k_FinallyClauseContext_t;

// Constructor

/**
 * @memberof FinallyClauseContext
 */
k_FinallyClauseContext_t* k_FinallyClauseContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof FinallyClauseContext
 */
void k_FinallyClauseContext_delete(k_FinallyClauseContext_t* context);

// Children

/**
 * @memberof FinallyClauseContext
 */
void k_FinallyClauseContext_getChildren(k_FinallyClauseContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_FINALLY_CLAUSE_CONTEXT_H */
