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

// Sunday, November 26, 2017

#ifndef COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_WALKER_H
#define COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_WALKER_H

#include <kush/Configuration.h>
#include <kush/ast/ASTListener.h>
#include <kush/ast/ASTNode.h>

/*******************************************************************************
 * ASTWalker                                                                   *
 *******************************************************************************/

/**
 * @memberof ASTWalker
 */
void k_ASTWalker_walk(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTWalker
 */
void k_ASTWalker_enterRule(k_ASTListener_t* listener, k_ASTNode_t* node);

/**
 * @memberof ASTWalker
 */
void k_ASTWalker_exitRule(k_ASTListener_t* listener, k_ASTNode_t* node);

#endif /* COM_ONECUBE_ZEN_COMPILER_COMPILER_AST_WALKER_H */