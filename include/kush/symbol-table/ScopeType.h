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

// Sunday, January 07, 2018

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SCOPE_TYPE_H
#define KUSH_COMPILER_SYMBOL_TABLE_SCOPE_TYPE_H

/*******************************************************************************
 * ScopeType                                                                   *
 *******************************************************************************/

/**
 * @class ScopeType
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
enum k_ScopeType_t {
    KUSH_SCOPE_COMPILATION_UNIT,
    KUSH_SCOPE_ANNOTATION,
    KUSH_SCOPE_CLASS,
    KUSH_SCOPE_FUNCTION,
    KUSH_SCOPE_ENUMERATION,
    KUSH_SCOPE_LOCAL
};

/**
 * @memberof ScopeType
 */
typedef enum k_ScopeType_t k_ScopeType_t;

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SCOPE_TYPE_H */
