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

#ifndef COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_IF_CLAUSE_CONTEXT_H
#define COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_IF_CLAUSE_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * IfClauseContext                                                             *
 *******************************************************************************/

/**
 * @class IfClauseContext
 * @ingroup zen_compiler_ast
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct zen_IfClauseContext_t {
    zen_ASTNode_t* m_node;
    zen_ASTNode_t* m_expression;
    zen_ASTNode_t* m_statementSuite;
};

/**
 * @memberof IfClauseContext
 */
typedef struct zen_IfClauseContext_t zen_IfClauseContext_t;

// Constructor

/**
 * @memberof IfClauseContext
 */
zen_IfClauseContext_t* zen_IfClauseContext_new(zen_ASTNode_t* node);

// Destructor

/**
 * @memberof IfClauseContext
 */
void zen_IfClauseContext_delete(zen_IfClauseContext_t* context);

// Children

/**
 * @memberof IfClauseContext
 */
void zen_IfClauseContext_getChildren(zen_IfClauseContext_t* context, jtk_ArrayList_t* children);

#endif /* COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_IF_CLAUSE_CONTEXT_H */