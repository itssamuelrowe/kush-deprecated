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

#ifndef KUSH_COMPILER_AST_CONTEXT_LOGICAL_AND_EXPRESSION_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_LOGICAL_AND_EXPRESSION_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * LogicalAndExpressionContext                                                 *
 *******************************************************************************/

/**
 * @class LogicalAndExpressionContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_LogicalAndExpressionContext_t {
    k_ASTNode_t* m_node;

    /**
     * The inclusiveOrExpression node on the left side of the operator.
     */
    k_ASTNode_t* m_inclusiveOrExpression;

    /**
     * The inclusiveOrExpression nodes on the right side of the operator.
     */
    jtk_ArrayList_t* m_inclusiveOrExpressions;
};

/**
 * @memberof LogicalAndExpressionContext
 */
typedef struct k_LogicalAndExpressionContext_t k_LogicalAndExpressionContext_t;

// Constructor

/**
 * @memberof LogicalAndExpressionContext
 */
k_LogicalAndExpressionContext_t* k_LogicalAndExpressionContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof LogicalAndExpressionContext
 */
void k_LogicalAndExpressionContext_delete(k_LogicalAndExpressionContext_t* context);

// Children

/**
 * @memberof LogicalAndExpressionContext
 */
void k_LogicalAndExpressionContext_getChildren(k_LogicalAndExpressionContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_LOGICAL_AND_EXPRESSION_CONTEXT_H */
