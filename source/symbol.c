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

#include <kush/Symbol.h>
#include <kush/Token.h>
#include <jtk/core/CString.h>

/*******************************************************************************
 * Symbol                                                                      *
 *******************************************************************************/

k_Symbol_t* k_Symbol_new(k_SymbolCategory_t category,
    k_ASTNode_t* identifier, Scope* parent) {
    k_Symbol_t* symbol = allocate(k_Symbol_t, 1);
    symbol->category = category;
    symbol->identifier = identifier;
    symbol->parent = parent;
    symbol->modifiers = 0;
    symbol->ticket = (parent != NULL)? parent->nextTicket++ : -1;
    symbol->index = -1;
    symbol->flags = 0;
    if (identifier != NULL) {
        Token* token = (Token*)identifier->context;
        symbol->name = token->text;
        symbol->nameSize = token->length;
    }

    if (category == ZEN_SYMBOL_CATEGORY_FUNCTION) {
        k_FunctionSymbol_initialize(&symbol->context.asFunction);
    }
    else if (category == ZEN_SYMBOL_CATEGORY_CLASS) {
        k_ClassSymbol_initialize(&symbol->context.asClass);
    }

    return symbol;
}

k_Symbol_t* k_Symbol_forConstant(k_ASTNode_t* identifier,
    Scope* parent) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_CONSTANT, identifier, parent);
}

k_Symbol_t* k_Symbol_forVariable(k_ASTNode_t* identifier,
    Scope* parent) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_VARIABLE, identifier, parent);
}

k_Symbol_t* k_Symbol_forFunction(k_ASTNode_t* identifier,
    Scope* parent) {
    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_FUNCTION,
        identifier, parent);
    return symbol;
}

k_Symbol_t* k_Symbol_forClass(k_ASTNode_t* identifier,
    Scope* parent, Scope* classScope, const uint8_t* name,
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

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_CLASS, identifier, parent);
    k_ClassSymbol_t* classSymbol = &symbol->context.asClass;
    classSymbol->qualifiedName = qualifiedName;
    classSymbol->qualifiedNameSize = qualifiedNameSize;
    classSymbol->descriptor = descriptor;
    classSymbol->descriptorSize = qualifiedNameSize;
    classSymbol->classScope = classScope;

    return symbol;
}

k_Symbol_t* k_Symbol_forClassAlt(Scope* classScope, const uint8_t* descriptor,
    int32_t descriptorSize) {
    uint8_t* qualifiedName = jtk_CString_newEx(descriptor,
        descriptorSize);
    jtk_Arrays_replace_b(qualifiedName, descriptorSize, '/', '.');

    k_Symbol_t* symbol = k_Symbol_new(ZEN_SYMBOL_CATEGORY_CLASS, NULL, NULL);
    k_ClassSymbol_t* classSymbol = &symbol->context.asClass;
    classSymbol->qualifiedName = qualifiedName;
    classSymbol->qualifiedNameSize = descriptorSize;
    classSymbol->descriptor = descriptor;
    classSymbol->descriptorSize = descriptorSize;
    classSymbol->classScope = classScope;

    return symbol;
}

k_Symbol_t* k_Symbol_forLabel(k_ASTNode_t* identifier,
    Scope* parent) {
    return k_Symbol_new(ZEN_SYMBOL_CATEGORY_LABEL, identifier, parent);
}

k_Symbol_t* k_Symbol_forExternal(k_ASTNode_t* identifier,
    Scope* parent, k_Symbol_t* other) {
    k_Symbol_t* result = k_Symbol_new(ZEN_SYMBOL_CATEGORY_EXTERNAL,
        identifier, parent);
    result->context.asExternal = other;

    return result;
}

void k_Symbol_delete(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    if (symbol->category == ZEN_SYMBOL_CATEGORY_FUNCTION) {
        k_FunctionSymbol_destroy(&symbol->context.asFunction);
    }
    else if (symbol->category == ZEN_SYMBOL_CATEGORY_CLASS) {
        k_ClassSymbol_destroy(&symbol->context.asClass);
    }

    deallocate(symbol);
}

// Category

k_SymbolCategory_t k_Symbol_getCategory(k_Symbol_t* symbol) {
    return symbol->category;
}

bool k_Symbol_isEnumeration(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_ENUMERATION);
}

bool k_Symbol_isEnumerate(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_ENUMERATE);
}

bool k_Symbol_isFunction(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_FUNCTION);
}

bool k_Symbol_isConstant(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_CONSTANT);
}

bool k_Symbol_isVariable(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_VARIABLE);
}

bool k_Symbol_isClass(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_CLASS);
}

bool k_Symbol_isExternal(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return (symbol->category == ZEN_SYMBOL_CATEGORY_EXTERNAL);
}

Scope* k_Symbol_getEnclosingScope(k_Symbol_t* symbol) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");
    return symbol->parent;
}

k_ASTNode_t* k_Symbol_getIdentifier(k_Symbol_t* symbol) {
    return symbol->identifier;
}

/* Modifier */

void k_Symbol_addModifiers(k_Symbol_t* symbol, uint32_t modifiers) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    symbol->modifiers |= modifiers;
}

bool k_Symbol_hasModifiers(k_Symbol_t* symbol, uint32_t modifiers) {
    jtk_Assert_assertObject(symbol, "The specified symbol is null.");

    return (symbol->modifiers & modifiers) == modifiers;
}

bool k_Symbol_isStatic(k_Symbol_t* symbol) {
    return (symbol->modifiers & ZEN_MODIFIER_STATIC) != 0;
}

k_FunctionSignature_t* k_Symbol_getFunctionSignature(k_Symbol_t* symbol,
    int32_t argumentCount) {
    k_FunctionSymbol_t* functionSymbol = &symbol->context.asFunction;
    if ((argumentCount >= functionSymbol->parameterThreshold) &&
        (functionSymbol->parameterThreshold >= 0)) {
        argumentCount = functionSymbol->parameterThreshold;
    }

    k_FunctionSignature_t* result = NULL;
    int32_t i;
    int32_t count = jtk_ArrayList_getSize(functionSymbol->signatures);
    for (i = 0; i < count; i++) {
        k_FunctionSignature_t* signature = (k_FunctionSignature_t*)
            jtk_ArrayList_getValue(functionSymbol->signatures, i);
        int32_t parameterCount = signature->fixedParameterCount; // jtk_ArrayList_getSize(signature->fixedParameters);
        if (parameterCount == argumentCount) {
            result = signature;
            break;
        }
    }
    return result;
}

k_FunctionSignature_t* k_Symbol_getFunctionSignatureEx(k_Symbol_t* symbol,
    const uint8_t* descriptor, int32_t descriptorSize) {
    k_FunctionSymbol_t* functionSymbol = &symbol->context.asFunction;
    k_FunctionSignature_t* result = NULL;
    int32_t i;
    int32_t count = jtk_ArrayList_getSize(functionSymbol->signatures);
    for (i = 0; i < count; i++) {
        k_FunctionSignature_t* signature = (k_FunctionSignature_t*)
            jtk_ArrayList_getValue(functionSymbol->signatures, i);
        if (jtk_CString_equals(signature->descriptor, signature->descriptorSize,
            descriptor, descriptorSize)) {
            result = signature;
            break;
        }
    }
    return result;
}
