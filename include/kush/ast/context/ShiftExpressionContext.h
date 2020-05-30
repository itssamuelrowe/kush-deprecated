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

#ifndef KUSH_COMPILER_AST_CONTEXT_SHIFT_EXPRESSION_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_SHIFT_EXPRESSION_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ShiftExpressionContext                                                      *
 *******************************************************************************/

/**
 * @class ShiftExpressionContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_ShiftExpressionContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_additiveExpression;
    jtk_ArrayList_t* m_additiveExpressions;
};

/**
 * @memberof ShiftExpressionContext
 */
typedef struct k_ShiftExpressionContext_t k_ShiftExpressionContext_t;

// Constructor

/**
 * @memberof ShiftExpressionContext
 */
k_ShiftExpressionContext_t* k_ShiftExpressionContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof ShiftExpressionContext
 */
void k_ShiftExpressionContext_delete(k_ShiftExpressionContext_t* context);

// Children

/**
 * @memberof ShiftExpressionContext
 */
void k_ShiftExpressionContext_getChildren(k_ShiftExpressionContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_SHIFT_EXPRESSION_CONTEXT_H */
