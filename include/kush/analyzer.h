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

#ifndef KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H
#define KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H

#include <kush/Configuration.h>
#include <kush/compiler.h>
#include <kush/ContextType.h>
#include <kush/symbol-table/Scope.h>
#include <kush/symbol-table/ScopeType.h>
#include <kush/symbol-table/SymbolTable.h>

/*******************************************************************************
 * SymbolDefinitionListener                                                    *
 *******************************************************************************/

/**
 * @class Analyzer
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct Analyzer {
    k_Compiler_t* compiler;
    k_ASTListener_t* astListener;
    k_SymbolTable_t* symbolTable;
    k_ASTAnnotations_t* scopes;
    const uint8_t* package;
    int32_t packageSize;
    k_ASTNodeType_t mainComponent;
    bool classPrepared;
    uint8_t* className;
    int32_t classNameSize;
};

typedef struct Analyzer Analyzer;

#endif /* KUSH_COMPILER_SYMBOL_TABLE_SYMBOL_DEFINITION_LISTENER_H */
