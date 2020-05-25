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

#ifndef COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_CONTINUE_STATEMENT_CONTEXT_H
#define COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_CONTINUE_STATEMENT_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ContinueStatementContext                                                    *
 *******************************************************************************/

/**
 * @class ContinueStatementContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_ContinueStatementContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_identifier;
};

/**
 * @memberof ContinueStatementContext
 */
typedef struct k_ContinueStatementContext_t k_ContinueStatementContext_t;

// Constructor

/**
 * @memberof ContinueStatementContext
 */
k_ContinueStatementContext_t* k_ContinueStatementContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof ContinueStatementContext
 */
void k_ContinueStatementContext_delete(k_ContinueStatementContext_t* context);

// Children

/**
 * @memberof ContinueStatementContext
 */
void k_ContinueStatementContext_getChildren(k_ContinueStatementContext_t* context, jtk_ArrayList_t* children);

#endif /* COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_CONTINUE_STATEMENT_CONTEXT_H */