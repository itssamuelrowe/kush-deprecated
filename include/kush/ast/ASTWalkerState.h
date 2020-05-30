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

#ifndef KUSH_COMPILER_AST_AST_WALKER_STATE_H
#define KUSH_COMPILER_AST_AST_WALKER_STATE_H

/*******************************************************************************
 * ASTWalker                                                                   *
 *******************************************************************************/

/**
 * @class ASTWalker
 * @ingroup k_compiler_ast
 * @author Samuel Rowe <samuelrowe1999@gmail.com>
 * @since KUSH 1.0
 */
enum k_ASTWalkerState_t {
    KUSH_AST_WALKER_STATE_INVALID,

    KUSH_AST_WALKER_STATE_SKIP_CHILDREN,
    KUSH_AST_WALKER_STATE_VISIT_CHILDREN,

    KUSH_AST_WALKER_STATE_SKIP_EVEN_CHILDREN,
    KUSH_AST_WALKER_STATE_VISIT_EVEN_CHILDREN,

    KUSH_AST_WALKER_STATE_SKIP_ODD_CHILDREN,
    KUSH_AST_WALKER_STATE_VISIT_ODD_CHILDREN,

    KUSH_AST_WALKER_STATE_SKIP_EXTREME_CHILDREN,
    KUSH_AST_WALKER_STATE_VISIT_EXTREME_CHILDREN,

    KUSH_AST_WALKER_STATE_VISIT_FIRST_CHILD,
    KUSH_AST_WALKER_STATE_SKIP_FIRST_CHILD,

    KUSH_AST_WALKER_STATE_VISIT_LAST_CHILD,
    KUSH_AST_WALKER_STATE_SKIP_LAST_CHILD,
};

/**
 * @memberof ASTWalker
 */
typedef enum k_ASTWalkerState_t k_ASTWalkerState_t;

#endif /* KUSH_COMPILER_AST_AST_WALKER_STATE_H */
