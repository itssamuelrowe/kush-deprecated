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

// Saturday, March 10, 2018

#ifndef KUSH_COMPILER_COMPILER_AST_AST_ANNOTATIONS_H
#define KUSH_COMPILER_COMPILER_AST_AST_ANNOTATIONS_H

#include <jtk/collection/map/HashMap.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ASTAnnotations                                                             *
 *******************************************************************************/

struct k_ASTAnnotations_t {
    jtk_HashMap_t* m_map;
};

typedef struct k_ASTAnnotations_t k_ASTAnnotations_t;

k_ASTAnnotations_t* k_ASTAnnotations_new();
void k_ASTAnnotations_delete(k_ASTAnnotations_t* annotations);
void* k_ASTAnnotations_get(k_ASTAnnotations_t* annotations, k_ASTNode_t* node);
void k_ASTAnnotations_put(k_ASTAnnotations_t* annotations, k_ASTNode_t* node, void* value);
void k_ASTAnnotations_remove(k_ASTAnnotations_t* annotations, k_ASTNode_t* node);

#endif /* KUSH_COMPILER_COMPILER_AST_AST_ANNOTATIONS_H */
