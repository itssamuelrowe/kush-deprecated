/*
 * Copyright 2017-2020 Samuel Rowe
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

// Sunday, November 26, 2017

#ifndef K_PARSER_CONTEXT_TYPE_H
#define K_PARSER_CONTEXT_TYPE_H

/*******************************************************************************
 * ContextType                                                                 *
 *******************************************************************************/

/**
 * @class ContextType
 * @ingroup zen_compiler_ast
 * @author Samuel Rowe
 * @since kush 0.1
 */
enum k_ContextType_t {

    K_CONTEXT_UNKNOWN,

    K_CONTEXT_COMPILATION_UNIT,

    K_CONTEXT_IMPORT_DECLARATION,

    K_CONTEXT_FUNCTION_DECLARATION,

    K_CONTEXT_FUNCTION_PARAMETER,

    K_CONTEXT_BLOCK_STATEMENT,

    K_CONTEXT_VARIABLE_DECLARATION,

    K_CONTEXT_VARIABLE_DECLARATOR,

    K_CONTEXT_BREAK_STATEMENT,

    K_CONTEXT_RETURN_STATEMENT,

    K_CONTEXT_THROW_STATEMENT,

    K_CONTEXT_IF_STATEMENT,

    K_CONTEXT_IF_CLAUSE,

    K_CONTEXT_ITERATIVE_STATEMENT,

    K_CONTEXT_TRY_STATEMENT,

    K_CONTEXT_CATCH_CLAUSE,

    K_CONTEXT_STRUCTURE_DECLARATION,

    K_CONTEXT_ASSIGNMENT_EXPRESSION,

    K_CONTEXT_CONDITIONAL_EXPRESSION,

    K_CONTEXT_LOGICAL_OR_EXPRESSION,

    K_CONTEXT_LOGICAL_AND_EXPRESSION,

    K_CONTEXT_INCLUSIVE_OR_EXPRESSION,

    K_CONTEXT_EXCLUSIVE_OR_EXPRESSION,

    K_CONTEXT_AND_EXPRESSION,

    K_CONTEXT_EQUALITY_EXPRESSION,

    K_CONTEXT_RELATIONAL_EXPRESSION,

    K_CONTEXT_SHIFT_EXPRESSION,

    K_CONTEXT_ADDITIVE_EXPRESSION,

    K_CONTEXT_MULTIPLICATIVE_EXPRESSION,

    K_CONTEXT_UNARY_EXPRESSION,

    K_CONTEXT_POSTFIX_EXPRESSION,

    K_CONTEXT_INITIALIZER_EXPRESSION,

    K_CONTEXT_ARRAY_EXPRESSION
};

/**
 * @memberof ContextType
 */
typedef enum k_ContextType_t k_ContextType_t;

#endif /* K_PARSER_CONTEXT_TYPE_H */