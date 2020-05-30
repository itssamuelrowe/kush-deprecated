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

#ifndef KUSH_COMPILER_COMPILER_AST_AST_PRINTER_H
#define KUSH_COMPILER_COMPILER_AST_AST_PRINTER_H

#include <kush/Configuration.h>
#include <kush/ast/ASTListener.h>

/*******************************************************************************
 * ASTPrinter                                                                  *
 *******************************************************************************/

/**
 * @class ASTPrinter
 * @ingroup k_compiler_ast
 * @author Samuel Rowe
 * @since KUSH 1.0
 */
struct k_ASTPrinter_t {
    k_ASTListener_t* m_astListener;
    int32_t m_depth;
};

/**
 * @memberof ASTPrinter
 */
typedef struct k_ASTPrinter_t k_ASTPrinter_t;

/**
 * @memberof ASTPrinter
 */
k_ASTPrinter_t* k_ASTPrinter_new();

/**
 * @memberof ASTPrinter
 */
void k_ASTPrinter_delete(k_ASTPrinter_t* listener);

/**
 * @memberof ASTPrinter
 */
k_ASTListener_t* k_ASTPrinter_getASTListener(k_ASTPrinter_t* listener);

/**
 * @memberof ASTPrinter
 */
void k_ASTPrinter_onEnterEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTPrinter
 */
void k_ASTPrinter_onExitEveryRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTPrinter
 */
void k_ASTPrinter_onVisitTerminal(k_ASTListener_t* listener, k_ASTNode_t* node);

#endif /* KUSH_COMPILER_COMPILER_AST_AST_PRINTER_H */
