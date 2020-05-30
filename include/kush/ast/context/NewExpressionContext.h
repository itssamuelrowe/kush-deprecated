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

// Tuesday, July 16, 2019

#ifndef KUSH_COMPILER_AST_CONTEXT_NEW_EXPRESSION_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_NEW_EXPRESSION_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * NewExpressionContext                                                        *
 *******************************************************************************/

/**
 * @class NewExpressionContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_NewExpressionContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_typeName;
    k_ASTNode_t* m_functionArguments;
};

/**
 * @memberof NewExpressionContext
 */
typedef struct k_NewExpressionContext_t k_NewExpressionContext_t;

// Constructor

/**
 * @memberof NewExpressionContext
 */
k_NewExpressionContext_t* k_NewExpressionContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof NewExpressionContext
 */
void k_NewExpressionContext_delete(k_NewExpressionContext_t* context);

// Children

/**
 * @memberof NewExpressionContext
 */
void k_NewExpressionContext_getChildren(k_NewExpressionContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_NEW_EXPRESSION_CONTEXT_H */
