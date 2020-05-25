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

#ifndef COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FUNCTION_ARGUMENTS_CONTEXT_H
#define COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FUNCTION_ARGUMENTS_CONTEXT_H

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * FunctionArgumentsContext                                                    *
 *******************************************************************************/

/**
 * @class FunctionArgumentsContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_FunctionArgumentsContext_t {
    k_ASTNode_t* m_node;
    k_ASTNode_t* m_expressions;
};

/**
 * @memberof FunctionArgumentsContext
 */
typedef struct k_FunctionArgumentsContext_t k_FunctionArgumentsContext_t;

// Constructor

/**
 * @memberof FunctionArgumentsContext
 */
k_FunctionArgumentsContext_t* k_FunctionArgumentsContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof FunctionArgumentsContext
 */
void k_FunctionArgumentsContext_delete(k_FunctionArgumentsContext_t* context);

// Children

/**
 * @memberof FunctionArgumentsContext
 */
void k_FunctionArgumentsContext_getChildren(k_FunctionArgumentsContext_t* context, jtk_ArrayList_t* children);

#endif /* COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FUNCTION_ARGUMENTS_CONTEXT_H */