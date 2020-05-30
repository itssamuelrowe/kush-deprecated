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

#ifndef KUSH_COMPILER_AST_CONTEXT_COMPILATION_UNIT_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_COMPILATION_UNIT_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * CompilationUnitContext                                                      *
 *******************************************************************************/

/**
 * @class CompilationUnitContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_CompilationUnitContext_t {
    /**
     * The node which represents this rule context within the AST.
     */
    k_ASTNode_t* m_node;
	jtk_ArrayList_t* m_importDeclarations; /* <k_ASTNode_t*> */
	jtk_ArrayList_t* m_annotatedComponentDeclarations; /* <k_ASTNode_t*> */
};

/**
 * @memberof CompilationUnitContext
 */
typedef struct k_CompilationUnitContext_t k_CompilationUnitContext_t;

// Constructor

/**
 * @memberof CompilationUnitContext
 */
k_CompilationUnitContext_t* k_CompilationUnitContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof CompilationUnitContext
 */
void k_CompilationUnitContext_delete(k_CompilationUnitContext_t* context);

// Children

/**
 * @memberof CompilationUnitContext
 */
void k_CompilationUnitContext_getChildren(k_CompilationUnitContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_COMPILATION_UNIT_CONTEXT_H */
