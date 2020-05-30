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

// Saturday, November 25, 2017

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_CATEGORY_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_CATEGORY_H

/*******************************************************************************
 * SymbolCategory                                                              *
 *******************************************************************************/

/**
 * @class SymbolCategory
 * @ingroup k_compiler_symbolTable
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
enum k_SymbolCategory_t {
    KUSH_SYMBOL_CATEGORY_ANNOTATION,
    KUSH_SYMBOL_CATEGORY_CLASS,
    KUSH_SYMBOL_CATEGORY_ENUMERATE,
    KUSH_SYMBOL_CATEGORY_ENUMERATION,
    KUSH_SYMBOL_CATEGORY_FUNCTION,
    KUSH_SYMBOL_CATEGORY_CONSTANT,
    KUSH_SYMBOL_CATEGORY_VARIABLE,
    // KUSH_SYMBOL_CATEGORY_CONSTRUCTOR,
    KUSH_SYMBOL_CATEGORY_LABEL,
    KUSH_SYMBOL_CATEGORY_EXTERNAL
};

/**
 * @memberof SymbolCategory
 */
typedef enum k_SymbolCategory_t k_SymbolCategory_t;

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_CATEGORY_H */
