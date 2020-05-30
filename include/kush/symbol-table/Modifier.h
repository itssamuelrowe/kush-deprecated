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

// Sunday, March 04, 2018

#ifndef KUSH_COMPILER_SYMBOL_TABLE_MODIFIER_H
#define KUSH_COMPILER_SYMBOL_TABLE_MODIFIER_H

#include <kush/Configuration.h>

/*******************************************************************************
 * Modifier                                                                    *
 *******************************************************************************/

/**
 * @class Modifier
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
enum k_Modifier_t {

    KUSH_MODIFIER_PUBLIC = 1 << 0,

    KUSH_MODIFIER_PRIVATE = 1 << 1,

    KUSH_MODIFIER_SECRET = 1 << 2,

    KUSH_MODIFIER_SYNTHETIC = 1 << 3,

    KUSH_MODIFIER_ABSTRACT = 1 << 4,

    KUSH_MODIFIER_FINAL = 1 << 5,

    KUSH_MODIFIER_STATIC = 1 << 6,

    KUSH_MODIFIER_NATIVE = 1 << 7,

    KUSH_MODIFIER_ANNOTATION = 1 << 8,

    KUSH_MODIFIER_ENUMERATION = 1 << 9,

    KUSH_MODIFIER_VARIABLE_PARAMETER = 1 << 10
};

/**
 * @memberof Modifier
 */
typedef enum k_Modifier_t k_Modifier_t;



#endif /* KUSH_COMPILER_SYMBOL_TABLE_MODIFIER_H */
