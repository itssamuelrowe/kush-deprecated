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

#ifndef COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FOR_PARAMETER_CONTEXT_H
#define COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FOR_PARAMETER_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ForParameterContext                                                        *
 *******************************************************************************/

/**
 * @class ForParameterContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_ForParameterContext_t {
    k_ASTNode_t* m_node;
    bool m_declaration;
    bool m_variable;
    k_ASTNode_t* m_identifier;
};

/**
 * @memberof ForParameterContext
 */
typedef struct k_ForParameterContext_t k_ForParameterContext_t;

// Constructor

/**
 * @memberof ForParameterContext
 */
k_ForParameterContext_t* k_ForParameterContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof ForParameterContext
 */
void k_ForParameterContext_delete(k_ForParameterContext_t* context);

// Children

/**
 * @memberof ForParameterContext
 */
void k_ForParameterContext_getChildren(k_ForParameterContext_t* context, jtk_ArrayList_t* children);

#endif /* COM_ONECUBE_ZEN_COMPILER_AST_CONTEXT_FOR_PARAMETER_CONTEXT_H */