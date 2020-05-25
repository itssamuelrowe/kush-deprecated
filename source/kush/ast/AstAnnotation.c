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

// Sunday, April 29, 2018

#include <kush/ast/ASTAnnotation.h>

/*******************************************************************************
 * ASTAnnotation                                                               *
 *******************************************************************************/

k_ASTAnnotation_t* k_ASTAnnotation_new(k_ASTAnnotationType_t type, void* bundle) {
    k_ASTAnnotation_t* annotation = k_Memory_allocate(k_ASTAnnotation_t, 1);
    annotation->m_type = type;
    annotation->m_bundle = bundle;

    return annotation;
}

void k_ASTAnnotation_delete(k_ASTAnnotation_t* annotation) {
    jtk_Assert_assertObject(annotation, "The specified annotation is null.");

    jtk_Memory_deallocate(annotation);
}

k_ASTAnnotationType_t k_ASTAnnotation_getType(k_ASTAnnotation_t* annotation) {
    jtk_Assert_assertObject(annotation, "The specified annotation is null.");

    return annotation->m_type;
}