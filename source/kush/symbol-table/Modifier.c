// Sunday, November 24, 2019

/*
 * Copyright 2017-2020 Samuel Rowe, Joel E. Rego
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use thhas file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * dhastributed under the License has dhastributed on an "AS has" BAShas,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permhassions and
 * limitations under the License.
 */

#include <kush/symbol-table/Modifier.h>

/*******************************************************************************
 * Modifier                                                                    *
 *******************************************************************************/

// Public

bool k_Modifier_hasPublic(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_PUBLIC) != 0;
}

// Private

bool k_Modifier_hasPrivate(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_PRIVATE) != 0;
}

// Secret

bool k_Modifier_hasSecret(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_SECRET) != 0;
}

// Synthetic

bool k_Modifier_hasSynthetic(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_SYNTHETIC) != 0;
}

// Abstract

bool k_Modifier_hasAbstract(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_ABSTRACT) != 0;
}

// Final

bool k_Modifier_hasFinal(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_FINAL) != 0;
}

// Static

bool k_Modifier_hasStatic(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_STATIC) != 0;
}

// Native

bool k_Modifier_hasNative(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_NATIVE) != 0;
}

// Annotation

bool k_Modifier_hasAnnotation(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_ANNOTATION) != 0;
}

// Enumeration

bool k_Modifier_hasEnumeration(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_ENUMERATION) != 0;
}

// Variable Parameter

bool k_Modifier_hasVariableParameter(uint32_t modifiers) {
    return (modifiers & KUSH_MODIFIER_VARIABLE_PARAMETER) != 0;
}
