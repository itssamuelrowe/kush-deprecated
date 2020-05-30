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

#ifndef KUSH_COMPILER_AST_CONTEXT_STATEMENT_SUITE_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_STATEMENT_SUITE_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * StatementSuiteContext                                                       *
 *******************************************************************************/

/**
 * @class StatementSuiteContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_StatementSuiteContext_t {
    k_ASTNode_t* m_node;
    /* In order to help the users read code easily, the simple statements were
     * removed from statement suites. Previously, a statement suite was either
     * a simple statement or a block consisting of a newline at the begining,
     * indentation and dedentation.
     */
    // k_ASTNode_t* m_simpleStatement;
    jtk_ArrayList_t* m_statements;
    int32_t m_scope;
};

/**
 * @memberof StatementSuiteContext
 */
typedef struct k_StatementSuiteContext_t k_StatementSuiteContext_t;

// Constructor

/**
 * @memberof StatementSuiteContext
 */
k_StatementSuiteContext_t* k_StatementSuiteContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof StatementSuiteContext
 */
void k_StatementSuiteContext_delete(k_StatementSuiteContext_t* context);

// Children

/**
 * @memberof StatementSuiteContext
 */
void k_StatementSuiteContext_getChildren(k_StatementSuiteContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_STATEMENT_SUITE_CONTEXT_H */
