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

// Sunday, April 28, 2018

#ifndef COM_ONECUBE_COM_ONECUBE_COM_ONECUBE_ZEN_COMPILER_COMPILER_COMPILER_AST_AST_ANNOTATION_H
#define COM_ONECUBE_COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_AST_ANNOTATION_H

#include <kush/Configuration.h>
#include <kush/ast/ASTAnnotationType.h>

/*******************************************************************************
 * ASTAnnotation                                                               *
 *******************************************************************************/

/**
 * @author Samuel Rowe
 * @since  zen 1.0
 */
struct k_ASTAnnotation_t {
    k_ASTAnnotationType_t m_type;
    void* m_bundle;
};

typedef struct k_ASTAnnotation_t k_ASTAnnotation_t;

k_ASTAnnotation_t* k_ASTAnnotation_new(k_ASTAnnotationType_t type, void* bundle);
void k_ASTAnnotation_delete(k_ASTAnnotation_t* annotation);

#endif /* COM_ONECUBE_COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_AST_ANNOTATION_H */