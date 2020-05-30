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

#ifndef KUSH_COMPILER_AST_CONTEXT_IMPORT_DECLARATION_CONTEXT_H
#define KUSH_COMPILER_AST_CONTEXT_IMPORT_DECLARATION_CONTEXT_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ImportDeclarationContext                                                    *
 *******************************************************************************/

/**
 * @class ImportDeclarationContext
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ImportDeclarationContext_t {
    k_ASTNode_t* m_node;
    jtk_ArrayList_t* m_identifiers; /* <k_ASTNode_t*> */
    bool m_wildcard;
};

/**
 * @memberof ImportDeclarationContext
 */
typedef struct k_ImportDeclarationContext_t k_ImportDeclarationContext_t;

// Constructor

/**
 * @memberof ImportDeclarationContext
 */
k_ImportDeclarationContext_t* k_ImportDeclarationContext_new(k_ASTNode_t* node);

// Destructor

/**
 * @memberof ImportDeclarationContext
 */
void k_ImportDeclarationContext_delete(k_ImportDeclarationContext_t* context);

// Children

/**
 * @memberof ImportDeclarationContext
 */
void k_ImportDeclarationContext_getChildren(k_ImportDeclarationContext_t* context, jtk_ArrayList_t* children);

#endif /* KUSH_COMPILER_AST_CONTEXT_IMPORT_DECLARATION_CONTEXT_H */
