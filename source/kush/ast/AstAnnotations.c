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

#include <kush/ast/ASTAnnotations.h>
#include <jtk/core/PointerObjectAdapter.h>

/*******************************************************************************
 * ASTAnnotations                                                              *
 *******************************************************************************/

k_ASTAnnotations_t* k_ASTAnnotations_new() {
    k_ASTAnnotations_t* annotations = k_Memory_allocate(k_ASTAnnotations_t, 1);
    jtk_ObjectAdapter_t* keyAdapter = jtk_PointerObjectAdapter_getInstance();
    annotations->m_map = jtk_HashMap_new(keyAdapter, NULL);

    return annotations;
}

void k_ASTAnnotations_delete(k_ASTAnnotations_t* annotations) {
    jtk_Assert_assertObject(annotations, "The specified annotations is null.");

    jtk_HashMap_delete(annotations->m_map);
    jtk_Memory_deallocate(annotations);
}

void* k_ASTAnnotations_get(k_ASTAnnotations_t* annotations, k_ASTNode_t* node) {
    jtk_Assert_assertObject(annotations, "The specified annotations is null.");

    return jtk_HashMap_getValue(annotations->m_map, node);
}

void k_ASTAnnotations_put(k_ASTAnnotations_t* annotations, k_ASTNode_t* node, void* value) {
    jtk_Assert_assertObject(annotations, "The specified annotations is null.");

    if (!jtk_HashMap_putStrictly(annotations->m_map, (void*)node, value)) {
        printf("[internal error] AST node with multiple entries.\n");
    }
}

void k_ASTAnnotations_remove(k_ASTAnnotations_t* annotations, k_ASTNode_t* node) {
    jtk_Assert_assertObject(annotations, "The specified annotations is null.");

    jtk_HashMap_removeKey(annotations->m_map, node);
}