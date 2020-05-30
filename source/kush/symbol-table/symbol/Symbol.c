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

// Monday, January 08, 2018

#include <kush/symbol-table/Symbol.h>
#include <kush/lexer/Token.h>
#include <jtk/core/CString.h>

/*******************************************************************************
 * Symbol                                                                      *
 *******************************************************************************/

k_Symbol_t* k_Symbol_new(k_SymbolCategory_t category,
    k_ASTNode_t* identifier, k_Scope_t* enclosingScope) {
    k_Symbol_t* symbol = k_Memory_allocate(k_Symbol_t, 1);
    symbol->m_category = category;
    symbol->m_identifier = identifier;
    symbol->m_enclosingScope = enclosingScope;
    symbol->m_modifiers = 0;
    symbol->m_ticket = (enclosingScope != NULL)? enclosingScope->m_nextTicket++ : -1;
    symbol->m_index = -1;
    symbol->m_flags = 0;
    if (identifier != NULL) {
        k_Token_t* token = (k_Token_t*)identifier->m_context;
        symbol->m_name = token->m_text;
        symbol->m_nameSize = token->m_length;
    }

    if (category == ZEN_SYMBOL_CATEGORY_FUNCTION) {
        k_FunctionSymbol_initialize(&symbol->m_context.m_asFunction);
    }
    else if (category == ZEN_SYMBOL_CATEGORY_CLASS) {
        k_ClassSymbol_initialize(&symbol->m_context.m_asClass);
    }

    return symbol;
}

k_Symbol_t* k_Symbol_forConstant(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_CONSTANT, identifier, enclosingScope);
}

k_Symbol_t* k_Symbol_forVariable(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_VARIABLE, identifier, enclosingScope);
}

k_Symbol_t* k_Symbol_forFunction(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope) {
    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_FUNCTION,
        identifier, enclosingScope);
    return symbol;
}

k_Symbol_t* k_Symbol_forClass(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope, k_Scope_t* classScope, const uint8_t* name,
    int32_t nameSize, const uint8_t* package, int32_t packageSize) {
    uint8_t* qualifiedName = NULL;
    int32_t qualifiedNameSize = -1;
    if (package != NULL) {
        uint8_t* strings[] = {
            package,
            ".",
            name
        };
        int32_t sizes[] = {
            packageSize,
            1,
            nameSize
        };
        qualifiedName = jtk_CString_joinAll(strings, sizes, 3, &qualifiedNameSize);
    }
    else {
        qualifiedName = jtk_CString_newEx(name, nameSize);
        qualifiedNameSize = nameSize;
    }

    uint8_t* descriptor = jtk_CString_newEx(qualifiedName, qualifiedNameSize);
    jtk_Arrays_replace_b(descriptor, qualifiedNameSize, '.', '/');

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_CLASS, identifier, enclosingScope);
    k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
    classSymbol->m_qualifiedName = qualifiedName;
    classSymbol->m_qualifiedNameSize = qualifiedNameSize;
    classSymbol->m_descriptor = descriptor;
    classSymbol->m_descriptorSize = qualifiedNameSize;
    classSymbol->m_classScope = classScope;

    return symbol;
}

k_Symbol_t* k_Symbol_forClassAlt(k_Scope_t* classScope, const uint8_t* descriptor,
    int32_t descriptorSize) {
    uint8_t* qualifiedName = jtk_CString_newEx(descriptor,
        descriptorSize);
    jtk_Arrays_replace_b(qualifiedName, descriptorSize, '/', '.');

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_CLASS, NULL, NULL);
    k_ClassSymbol_t* classSymbol = &symbol->m_context.m_asClass;
    classSymbol->m_qualifiedName = qualifiedName;
    classSymbol->m_qualifiedNameSize = descriptorSize;
    classSymbol->m_descriptor = descriptor;
    classSymbol->m_descriptorSize = descriptorSize;
    classSymbol->m_classScope = classScope;

    return symbol;
}

k_Symbol_t* k_Symbol_forLabel(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_LABEL, identifier, enclosingScope);
}

k_Symbol_t* k_Symbol_forExternal(k_ASTNode_t* identifier,
    k_Scope_t* enclosingScope, k_Symbol_t* other) {
    k_Symbol_t* result = k_Symbol_new(ZEN_SYMBOL_CATEGORY_EXTERNAL,
        identifier, enclosingScope);
    result->m_context.m_asExternal = other;

    return result;
}

void k_Symbol_delete(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    if (symbol->m_category == ZEN_SYMBOL_CATEGORY_FUNCTION) {
        k_FunctionSymbol_destroy(&symbol->m_context.m_asFunction);
    }
    else if (symbol->m_category == ZEN_SYMBOL_CATEGORY_CLASS) {
        k_ClassSymbol_destroy(&symbol->m_context.m_asClass);
    }

    jtk_Memory_deallocate(symbol);
}

// Category

k_SymbolCategory_t k_Symbol_getCategory(k_Symbol_t* symbol) {
    return symbol->m_category;
}

bool k_Symbol_isEnumeration(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_ENUMERATION);
}

bool k_Symbol_isEnumerate(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_ENUMERATE);
}

bool k_Symbol_isFunction(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_FUNCTION);
}

bool k_Symbol_isConstant(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_CONSTANT);
}

bool k_Symbol_isVariable(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_VARIABLE);
}

bool k_Symbol_isClass(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_CLASS);
}

bool k_Symbol_isExternal(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->m_category == ZEN_SYMBOL_CATEGORY_EXTERNAL);
}

k_Scope_t* k_Symbol_getEnclosingScope(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return symbol->m_enclosingScope;
}

k_ASTNode_t* k_Symbol_getIdentifier(k_Symbol_t* symbol) {
    return symbol->m_identifier;
}

/* Modifier */

void k_Symbol_addModifiers(k_Symbol_t* symbol, uint32_t modifiers) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    symbol->m_modifiers |= modifiers;
}

bool k_Symbol_hasModifiers(k_Symbol_t* symbol, uint32_t modifiers) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return (symbol->m_modifiers & modifiers) == modifiers;
}

bool k_Symbol_isStatic(k_Symbol_t* symbol) {
    return (symbol->m_modifiers & ZEN_MODIFIER_STATIC) != 0;
}

k_FunctionSignature_t* k_Symbol_getFunctionSignature(k_Symbol_t* symbol,
    int32_t argumentCount) {
    k_FunctionSymbol_t* functionSymbol = &symbol->m_context.m_asFunction;
    if ((argumentCount >= functionSymbol->m_parameterThreshold) &&
        (functionSymbol->m_parameterThreshold >= 0)) {
        argumentCount = functionSymbol->m_parameterThreshold;
    }

    k_FunctionSignature_t* result = NULL;
    int32_t i;
    int32_t count = jtk_ArrayList_getSize(functionSymbol->m_signatures);
    for (i = 0; i < count; i++) {
        k_FunctionSignature_t* signature = (k_FunctionSignature_t*)
            jtk_ArrayList_getValue(functionSymbol->m_signatures, i);
        int32_t parameterCount = signature->m_fixedParameterCount; // jtk_ArrayList_getSize(signature->m_fixedParameters);
        if (parameterCount == argumentCount) {
            result = signature;
            break;
        }
    }
    return result;
}

k_FunctionSignature_t* k_Symbol_getFunctionSignatureEx(k_Symbol_t* symbol,
    const uint8_t* descriptor, int32_t descriptorSize) {
    k_FunctionSymbol_t* functionSymbol = &symbol->m_context.m_asFunction;
    k_FunctionSignature_t* result = NULL;
    int32_t i;
    int32_t count = jtk_ArrayList_getSize(functionSymbol->m_signatures);
    for (i = 0; i < count; i++) {
        k_FunctionSignature_t* signature = (k_FunctionSignature_t*)
            jtk_ArrayList_getValue(functionSymbol->m_signatures, i);
        if (jtk_CString_equals(signature->m_descriptor, signature->m_descriptorSize,
            descriptor, descriptorSize)) {
            result = signature;
            break;
        }
    }
    return result;
}
