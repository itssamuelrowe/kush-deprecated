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

// Thursday, October 29, 2019

#ifndef KUSH_COMPILER_AST_CONTEXT_WITH_PARAMETER_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_WITH_PARAMETER_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * WithParameterContext                                                        *
 *******************************************************************************/

/**
 * @class WithParameterContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_WithParameterContext_t {
    k_ASTNode_t* m_node;
    bool m_variable;
    k_ASTNode_t* m_identifier;
    k_ASTNode_t* m_expression;
};

/**
 * @memberof WithParameterContext
 */
typedef struct k_WithParameterContext_t k_WithParameterContext_t;

// Constructor

/**
 * @memberof WithParameterContext
 */
k_WithParameterContext_t* k_WithParameterContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof WithParameterContext
 */
void k_WithParameterContext_delete(k_WithParameterContext_t* context);

// Children

/**
 * @memberof WithParameterContext
 */
void k_WithParameterContext_getChildren(k_WithParameterContext_t* context,
    jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_WITH_PARAMETER_CONTEXT_H */
