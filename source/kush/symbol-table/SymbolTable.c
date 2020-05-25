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

#include <jtk/collection/stack/LinkedStack.h>

#include <kush/Compiler.h>
#include <kush/symbol-table/SymbolTable.h>

k_SymbolTable_t* k_SymbolTable_new(k_Compiler_t* compiler) {
    k_SymbolTable_t* symbolTable = k_Memory_allocate(k_SymbolTable_t, 1);
    symbolTable->m_currentScope = NULL;
    symbolTable->m_compiler = compiler;

    return symbolTable;
}

void k_SymbolTable_delete(k_SymbolTable_t* symbolTable) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");

    jtk_Memory_deallocate(symbolTable);
}

void k_SymbolTable_setCurrentScope(k_SymbolTable_t* symbolTable, k_Scope_t* currentScope) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");

    jtk_Logger_t* logger = symbolTable->m_compiler->m_logger;
    jtk_Logger_debug(logger, "<enter> %s", k_Scope_getName(currentScope));
    symbolTable->m_currentScope = currentScope;
}

k_Scope_t* k_SymbolTable_getCurrentScope(k_SymbolTable_t* symbolTable) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");
    return symbolTable->m_currentScope;
}

void k_SymbolTable_invalidateCurrentScope(k_SymbolTable_t* symbolTable) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");

    jtk_Logger_t* logger = symbolTable->m_compiler->m_logger;
    jtk_Logger_debug(logger, "<exit> %s", k_Scope_getName(symbolTable->m_currentScope));
    symbolTable->m_currentScope = k_Scope_getEnclosingScope(symbolTable->m_currentScope);
}

void k_SymbolTable_define(k_SymbolTable_t* symbolTable, k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    k_Scope_define(symbolTable->m_currentScope, symbol);
}

/* Resolve */

k_Symbol_t* k_SymbolTable_resolve(k_SymbolTable_t* symbolTable, const uint8_t* identifier) {
    jtk_Assert_assertObject(symbolTable, "The specified symbol table is null.");
    jtk_Assert_assertObject(identifier, "The specified identifier is null.");

    k_Scope_t* scope = symbolTable->m_currentScope;
    k_Symbol_t* symbol = NULL;
    while (scope != NULL) {
        symbol = k_Scope_resolve(scope, identifier);
        if (symbol != NULL) {
            break;
        }
        else {
            scope = k_Scope_getEnclosingScope(scope);
        }
    }

    if ((symbol != NULL) && k_Symbol_isExternal(symbol)) {
        symbol = symbol->m_context.m_asExternal;
    }

    return symbol;
}