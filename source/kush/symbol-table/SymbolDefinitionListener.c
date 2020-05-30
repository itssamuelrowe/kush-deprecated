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

#include <jtk/core/CString.h>
#include <jtk/collection/stack/LinkedStack.h>

#include <kush/ast/ASTNode.h>
#include <kush/lexer/Token.h>
#include <kush/symbol-table/FunctionSignature.h>
#include <kush/symbol-table/FunctionSymbol.h>
#include <kush/symbol-table/SymbolDefinitionListener.h>
#include <kush/symbol-table/SymbolTable.h>

/*
 * As new features are implemented, the source code of a project grows in
 * complexity. It becomes hard to maintain the source code when everything is
 * crammed into a single source file. Therefore, an intuitive feature of a
 * programming language is to allow programmers to split their source code
 * into multiple files. The symbols defined in these source files can be imported
 * and exported outside a given source file or compilation unit as needed.
 *
 * In KUSH, the import statement allows a compilation unit to refer to external
 * entities such as functions and classes. Without the use of the import statement,
 * the only way to refer to an entity outside the current compilatiin unit is to
 * use a fully qualified name. Further, all top-level entities declared in KUSH
 * are exported by default. As of this writing, there is no way to override this
 * behaviour.
 *
 * Albeit the import/export mechanism is complicated, it is an interesting topic.
 * The following article describes the import/export mechanism employed by the
 * KUSH compiler behind the scenes.
 *
 * ## Entity Forms
 *
 * A compilation unit can import entities that are either compiled or non-compiled.
 * (Note that the design assumes that the form of an entity is transparent
 * to the end user, i.e., the programmer.)
 *  1. Entities whose equivalent ".feb" files exist on the filsystem in the
 *     entity lookup directories specified to the compiler. These entities are
 *     referred to as compiled entities.
 *  2. Entities that are part of the current compilation batch. These entites
 *     are referred to as non-compiled entities.
 *
 * ## The Global Symbol Cache
 *
 * Given we are dealing with two forms of entities (compiled and non-compiled),
 * the design should allow for an abstraction which allows layers above the
 * import mechanism to work seamlessly without having to worry about an entities
 * form. In order to implement such an abstraction, a central repository known
 * as the global symbol cache is maintained by the compiler.
 *
 * The global symbol cache holds symbols which represent external entities. Here,
 * an external entity refers to any entity that is defined outside a compilation
 * unit. Internally, the global symbol cache uses a hash map to keep track of
 * the registered symbols. The various phases of the compiler can request the
 * global symbol cache to acquire symbols corresponding to entities. However,
 * symbols exported from compilation units that are part of the current compilation
 * batch are not available until the definition phase of the compiler is over.
 * In other words, non-compiled entities are not available until the definition
 * phase is complete.
 *
 * The global symbol cache satifies a symbol request in one of the following ways:
 * 1. If the requested symbol is found in the internal hash map, it is
 *    immediately returned.
 * 2. If the requested symbol is not found in the internal hash map, the cache
 *    searches the entity lookup directories to find a binary entity. If found,
 *    it loads the binary entity using the embedded binary entity loader and a
 *    corresponding symbol is created, inserted in the internal hash map, and
 *    returned. Otherwise, the request fails and null is returned. This allows
 *    the compiler to load external entities from their binary counterpart without
 *    the requesting party to manually deal with external entities.
 *
 *    What about non-compiled external entities? We know that such entities are
 *    part of the current compilation batch. The compiler can take advantage of
 *    this fact. During the definition phase, the compiler registers any symbol
 *    that is considered as an external symbol to the global symbol cache. This allows
 *    compilation units in the current compilation batch to reference symbols
 *    declared in another compilation unit. For example, consider two compilation
 *    units `BirdWatch.KUSH` and `Sparrow.KUSH` in the current compilation batch which
 *    declare the `BirdWatch` and `Sparrow` classes, respectively.
 *    Further, assume that `BirdWatch.KUSH` imports `Sparrow`. During the definition
 *    phase corresponding to `Sparrow.KUSH`, a class symbol for `Sparrow` is registered
 *    in the global cache. This allows `BirdWatch` to refer to the external entity
 *    `Sparrow` which is part of the current compilation batch.
 *
 * ## Phases of the Compiler
 *
 * For multiple input source files, the compiler takes a source file and subjects
 * it to various phases in a parallel sequence, where the phases include, lexical
 * analysis, syntax analysis, semantic analysis (which is divided into the
 * defintion phase and the resolution phase), optimization, and code generation.
 * However, the same flow might not be suitable for the mechanism described here.
 * Why? If you think about it, the previous flow does not provide an oppurutunity
 * to the compiler to register non-compiled entities. In other words, the phases
 * of the compiler are specific  to each compilation unit which prevents exchange
 * of symbols between compilation units.
 *
 * Therefore, in order to accomodate the mechanism described here, the flow of
 * the compiler has to be altered. In the older flow, a source file was not
 * processed until the previous source file was completely processed, i.e.,
 * subjected to all the phases from lexical analysis to code generation. In the
 * new flow, all the input source files are subjected to a single phase before
 * moving on to the next phase. This allows the compiler to mediate the exchange
 * of symbols between compilation units.
 */

k_SymbolDefinitionListener_t* k_SymbolDefinitionListener_new(
    k_Compiler_t* compiler) {
    k_SymbolDefinitionListener_t* listener = k_Memory_allocate(k_SymbolDefinitionListener_t, 1);
    listener->m_compiler = compiler;
    listener->m_astListener = k_ASTListener_newWithContext(listener);
    listener->m_symbolTable = NULL;
    listener->m_scopes = NULL;
    listener->m_package = NULL;
    listener->m_packageSize = -1;
    listener->m_mainComponent = KUSH_AST_NODE_TYPE_UNKNOWN;
    listener->m_classPrepared = false;
    listener->m_className = NULL;
    listener->m_classNameSize = 0;

    k_ASTListener_t* astListener = listener->m_astListener;

    astListener->m_onEnterCompilationUnit = k_SymbolDefinitionListener_onEnterCompilationUnit;
    astListener->m_onExitCompilationUnit = k_SymbolDefinitionListener_onExitCompilationUnit;
    astListener->m_onEnterFunctionDeclaration = k_SymbolDefinitionListener_onEnterFunctionDeclaration;
    astListener->m_onExitFunctionDeclaration = k_SymbolDefinitionListener_onExitFunctionDeclaration;
    astListener->m_onEnterStatementSuite = k_SymbolDefinitionListener_onEnterStatementSuite;
    astListener->m_onExitStatementSuite = k_SymbolDefinitionListener_onExitStatementSuite;
    astListener->m_onEnterVariableDeclaration = k_SymbolDefinitionListener_onEnterVariableDeclaration;
    astListener->m_onEnterConstantDeclaration = k_SymbolDefinitionListener_onEnterConstantDeclaration;
    astListener->m_onEnterLabelClause = k_SymbolDefinitionListener_onEnterLabelClause;
    astListener->m_onEnterForParameter = k_SymbolDefinitionListener_onEnterForParameters;
    astListener->m_onEnterTryStatement = k_SymbolDefinitionListener_onEnterTryStatement;
    astListener->m_onEnterClassDeclaration = k_SymbolDefinitionListener_onEnterClassDeclaration;
    astListener->m_onExitClassDeclaration = k_SymbolDefinitionListener_onExitClassDeclaration;
    astListener->m_onEnterEnumerationDeclaration = k_SymbolDefinitionListener_onEnterEnumerationDeclaration;
    astListener->m_onExitEnumerationDeclaration = k_SymbolDefinitionListener_onExitEnumerationDeclaration;
    astListener->m_onEnterEnumerate = k_SymbolDefinitionListener_onEnterEnumerate;
    astListener->m_onExitEnumerate = k_SymbolDefinitionListener_onExitEnumerate;

    return listener;
}

void k_SymbolDefinitionListener_delete(k_SymbolDefinitionListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");

    k_ASTListener_delete(listener->m_astListener);
    jtk_Memory_deallocate(listener);
}

k_ASTListener_t* k_SymbolDefinitionListener_getASTListener(k_SymbolDefinitionListener_t* listener) {
    jtk_Assert_assertObject(listener, "The specified listener is null.");
    return listener->m_astListener;
}

void k_SymbolDefinitionListener_reset(
    k_SymbolDefinitionListener_t* listener,
    k_SymbolTable_t* symbolTable, k_ASTAnnotations_t* scopes,
    const uint8_t* package, int32_t packageSize) {
    listener->m_symbolTable = symbolTable;
    listener->m_scopes = scopes;
    listener->m_package = package;
    listener->m_packageSize = packageSize;
    listener->m_mainComponent = KUSH_AST_NODE_TYPE_UNKNOWN;
    listener->m_classPrepared = false;

    if (listener->m_className != NULL) {
        jtk_CString_delete(listener->m_className);
        listener->m_className = NULL;
        listener->m_classNameSize = 0;
    }
}

/* compilationUnit */

void k_SymbolDefinitionListener_onEnterCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;

    /* The scopes are represented as an n-ary tree, where the root scope is
     * an instance of k_CompilationUnitScope_t.
     */
    k_Scope_t* scope = k_Scope_forCompilationUnit();
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
    k_ASTAnnotations_put(listener->m_scopes, node, scope);
}

void k_SymbolDefinitionListener_onExitCompilationUnit(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

/* functionDeclaration */

// TODO: constructorArguments

// TODO: Declare function even if it has no parameters.

void k_SymbolDefinitionListener_declareOverloadedFunction(
    k_SymbolDefinitionListener_t* listener, k_Symbol_t* symbol,
    jtk_ArrayList_t* fixedParameters, k_ASTNode_t* variableParameter,
    uint32_t modifiers, k_Token_t* reference) {
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_FunctionSymbol_t* functionSymbol = &symbol->m_context.m_asFunction;
    k_Token_t* identifierToken = (k_Token_t*)symbol->m_identifier->m_context;

    /* Determines whether the overloaded function is semantically erroneous. */
    bool error = false;

    /* If the enclosing scope is same as the current scope, we
     * need to verify the signature of the function being declared
     * to be unique.
     */

    /* Let X be a function. If an overload of function X, say Xi, has k
     * fixed parameters and a variable parameter, then the maximum number
     * of fixed parameters for any overload of function X is limited to k - 1.
     * Where, k is known as the parameter threshold of function X.
     */
    int32_t parameterThreshold = k_FunctionSymbol_getParameterThreshold(functionSymbol);

    /* If currently there is no parameter threshold, try to update the
     * threshold.
     */
    int32_t fixedParameterCount = (fixedParameters != NULL)? jtk_ArrayList_getSize(fixedParameters) : 0;
    if (parameterThreshold < 0) {
        /* Update the threshold if the function being declared has a variable
         * parameter.
         */
        if (variableParameter != NULL) {
            parameterThreshold = fixedParameterCount;
            k_FunctionSymbol_setParameterThreshold(functionSymbol, parameterThreshold);
        }
    }

    /* Retrieve the overloaded signatures of the function symbol. */
    jtk_ArrayList_t* signatures = k_FunctionSymbol_getSignatures(functionSymbol);
    /* Determine the number of signatures. */
    int32_t size = jtk_ArrayList_getSize(signatures);
    /* Iterate over the signatures to determine if the signature of the
     * function being declared to be unique, or not.
     */
    int32_t i;
    for (i = 0; i < size; i++) {
        k_FunctionSignature_t* signature = (k_FunctionSignature_t*)jtk_ArrayList_getValue(signatures, i);
        if ((signature->m_variableParameter != NULL) && (variableParameter != NULL)) {
            k_ErrorHandler_handleSemanticalError(errorHandler,
                listener, KUSH_ERROR_CODE_MULTIPLE_FUNCTION_OVERLOADS_WITH_VARIABLE_PARAMETER,
                (k_Token_t*)(variableParameter->m_context));

            error = true;
        }
        else {
            int32_t fixedParameterCount0 = jtk_ArrayList_getSize(signature->m_fixedParameters);
            /* Determine whether the function being declared was duplicately
             * overloaded, or not.
             */
            if ((fixedParameterCount0 == fixedParameterCount) &&
                (signature->m_variableParameter == NULL)) {
                k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                    KUSH_ERROR_CODE_DUPLICATE_FUNCTION_OVERLOAD, identifierToken);
                error = true;
                break;
            }
            else {
                if (variableParameter != NULL) {
                    /* The function being declared caused the update of parameter
                     * threshold. Make sure that the number of parameters is
                     * less than the parameter threshold for previously defined
                     * functions.
                     */
                    if (fixedParameterCount0 >= parameterThreshold) {
                        /* NOTE: The error report does not point to the actual error.
                         * Instead it points to the function currently being declared.
                         */
                        k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                            KUSH_ERROR_CODE_FUNCTION_DECLARATION_CAUSES_ANOTHER_FUNCTION_TO_EXCEED_PARAMETER_THRESHOLD,
                            reference);
                        error = true;
                    }
                }
                else {
                    /* The function being declared did not cause the update of parameter threshold.
                     * A parameter threshold may have existed before this event was fired. Make sure
                     * that the function obliges the parameter threshold.
                     */
                    if ((parameterThreshold >= 0) && (fixedParameterCount >= parameterThreshold)) {
                        k_ErrorHandler_handleSemanticalError(errorHandler, listener,
                            KUSH_ERROR_CODE_FUNCTION_DECLARATION_EXCEEDS_PARAMETER_THRESHOLD,
                            reference);
                        error = true;
                        break;
                    }
                }
            }
        }
    }

    if (!error) {
        /* The function being declared is an overload of an existing
         * function. Add the signature of the function.
         */
        k_FunctionSignature_t* signature = k_FunctionSignature_new(fixedParameters, variableParameter, modifiers);
        k_FunctionSymbol_addSignature(functionSymbol, signature);
    }
}

k_Symbol_t* k_SymbolDefinitionListener_declareFunction(k_SymbolTable_t* symbolTable,
    k_ASTNode_t* identifier, jtk_ArrayList_t* fixedParameters,
    k_ASTNode_t* variableParameter, uint16_t modifiers) {
    /* Create a member function symbol to store in the symbol table. */
    k_Symbol_t* symbol = k_Symbol_forFunction(identifier, symbolTable->m_currentScope);
    k_FunctionSymbol_t* functionSymbol = &symbol->m_context.m_asFunction;
    if (variableParameter != NULL) {
        int32_t parameterThreshold = jtk_ArrayList_getSize(fixedParameters);
        k_FunctionSymbol_setParameterThreshold(functionSymbol, parameterThreshold);
    }
    /* In order to enable the symbol table to store overloaded functions,
     * we employ function signatures. Add the function signature
     * corresponding to the function being declared to the newly
     * created member function symbol.
     */
    k_FunctionSignature_t* signature = k_FunctionSignature_new(fixedParameters,
        variableParameter, modifiers);
    k_FunctionSymbol_addSignature(functionSymbol, signature);

    /* Define the symbol in the symbol table. */
    k_SymbolTable_define(symbolTable, symbol);

    return symbol;
}

void k_SymbolDefinitionListener_initializeClassName(k_SymbolDefinitionListener_t* listener) {
    k_Compiler_t* compiler = listener->m_compiler;
    const uint8_t* fileName = (const uint8_t*)jtk_ArrayList_getValue(
        compiler->m_inputFiles, compiler->m_currentFileIndex);
    int32_t size = jtk_CString_getSize(fileName);
    int32_t slashIndex = jtk_CString_findLast_c(fileName, size, '/');
    int32_t dotIndex = jtk_CString_findLast_c(fileName, size, '.');
    listener->m_className = jtk_CString_substringEx(fileName, size, slashIndex + 1,
        dotIndex);
    listener->m_classNameSize = dotIndex - (slashIndex + 1);
}

void k_SymbolDefinitionListener_defineClass(k_SymbolDefinitionListener_t* listener) {
    k_Scope_t* scope = k_Scope_forClass(listener->m_symbolTable->m_currentScope);

    k_Symbol_t* symbol = k_Symbol_forClass(NULL,
        listener->m_symbolTable->m_currentScope, scope,
        listener->m_className, listener->m_classNameSize,
        listener->m_package, listener->m_packageSize);
    k_ClassSymbol_t* classSymbol = &symbol->m_context;
    scope->m_symbol = symbol;

    k_Scope_defineEx(listener->m_symbolTable->m_currentScope, listener->m_className,
        listener->m_classNameSize, symbol);
    k_Compiler_registerSymbol(listener->m_compiler, classSymbol->m_qualifiedName,
        classSymbol->m_qualifiedNameSize, symbol);

    // TODO: Is the symbol defined here destroyed?
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);

    listener->m_classPrepared = true;
}

void k_SymbolDefinitionListener_onEnterFunctionDeclaration(k_ASTListener_t* astListener,
    k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_FunctionDeclarationContext_t* functionDeclarationContext = (k_FunctionDeclarationContext_t*)node->m_context;

    /* Retrieve the identifier associated with the function declaration. */
    k_ASTNode_t* identifier = functionDeclarationContext->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;

    if ((listener->m_mainComponent != KUSH_AST_NODE_TYPE_CLASS_DECLARATION)
        && !listener->m_classPrepared) {
        /* Pure static classes do not inherit any superclasses. */
        k_SymbolDefinitionListener_initializeClassName(listener);
        k_SymbolDefinitionListener_defineClass(listener);
    }

    k_FunctionParametersContext_t* functionParametersContext =
        (k_FunctionParametersContext_t*)functionDeclarationContext->m_functionParameters->m_context;
    /* Retrieve the fixed parameters associated with the context of the
     * function arguments.
     */
    jtk_ArrayList_t* fixedParameters = functionParametersContext->m_fixedParameters;
    /* Retrieve the variable parameter associated with the context of the
     * function arguments, if any.
     */
    k_ASTNode_t* variableParameter = functionParametersContext->m_variableParameter;
    int32_t fixedParameterCount = jtk_ArrayList_getSize(fixedParameters);

    /* Retrieve the symbol table associated with this listener. */
    k_SymbolTable_t* symbolTable = listener->m_symbolTable;
    /* Retrieve the current scope of the symbol table. */
    k_Scope_t* currentScope = k_SymbolTable_getCurrentScope(symbolTable);

    if ((k_Token_getType(identifierToken) == KUSH_TOKEN_KEYWORD_STATIC) &&
        ((fixedParameterCount > 0) || (variableParameter != NULL))) {
        k_ErrorHandler_handleSemanticalError(errorHandler, listener,
            KUSH_ERROR_CODE_STATIC_INITIALIZER_WITH_PARAMETERS, identifierToken);
    }
    else {
        if (k_Scope_isClassScope(currentScope)) {
            /* Resolve the identifier within the scope of the compilation unit. */
            k_Symbol_t* symbol = k_SymbolTable_resolve(symbolTable, identifierToken->m_text);
            uint32_t modifiers = 0;

            /* When functions are declared in the compilation unit, a
             * synthetic class symbol is generated. Therefore, make sure
             * that the class scope is not synthetic.
             */
            if (node->m_parent->m_type == KUSH_AST_NODE_TYPE_CLASS_MEMBER) {
                k_ClassMemberContext_t* classMemberContext = (k_ClassMemberContext_t*)node->m_parent->m_context;
                int32_t modifierCount = jtk_ArrayList_getSize(classMemberContext->m_modifiers);
                int32_t i;
                for (i = 0; i < modifierCount; i++) {
                    k_ASTNode_t* modifier =
                        (k_ASTNode_t*)jtk_ArrayList_getValue(classMemberContext->m_modifiers, i);
                    k_Token_t* token = (k_Token_t*)modifier->m_context;
                    modifiers |= k_TokenType_toModifiers(token->m_type);
                }
            }
            else {
                /* NOTE: Functions declared in the compilation unit cannot have modifiers
                 * specified explicitly. However, by default they are all static.
                 */
                modifiers |= KUSH_TOKEN_KEYWORD_STATIC;
            }

            if (symbol != NULL) {
                /* If a symbol with the given identifier exists, make sure it is
                 * a function before overloading.
                 */
                if (k_Symbol_isFunction(symbol)) {
                    /* The resolved symbol is a function. Retrieve the corresponding
                     * function symbol.
                     */
                    k_FunctionSymbol_t* functionSymbol = &symbol->m_context.m_asFunction;
                    k_SymbolDefinitionListener_declareOverloadedFunction(listener,
                        symbol, fixedParameters, variableParameter, modifiers,
                        identifierToken);
                }
                else {
                    k_ErrorHandler_handleSemanticalError(errorHandler,
                        listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_FUNCTION,
                        identifierToken);
                }
            }
            else {
                k_SymbolDefinitionListener_declareFunction(listener->m_symbolTable, identifier,
                    fixedParameters, variableParameter, modifiers);
            }
        }
        else {
            printf("[internal error] Declaring function in unsuitable scope.\n");
        }
    }

    /* A function scope is pushed regardless of the declaration being erroneous. */

    k_Scope_t* scope = k_Scope_forFunction(symbolTable->m_currentScope);
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
    k_ASTAnnotations_put(listener->m_scopes, node, scope);

    int32_t i;
    for (i = 0; i < fixedParameterCount; i++) {
        k_ASTNode_t* parameter = (k_ASTNode_t*)jtk_ArrayList_getValue(fixedParameters, i);
        const uint8_t* parameterText = k_Token_getText((k_Token_t*)(parameter->m_context));
        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, parameterText);
        if (symbol != NULL) {
            k_ErrorHandler_handleSemanticalError(errorHandler,
                listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_PARAMETER,
                (k_Token_t*)(parameter->m_context));
        }
        else {
            k_Symbol_t* symbol = k_Symbol_forConstant(parameter,
                symbolTable->m_currentScope);
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }
    }

    if (variableParameter != NULL) {
        const uint8_t* parameterText = k_Token_getText((k_Token_t*)(variableParameter->m_context));
        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, parameterText);
        if (symbol != NULL) {
            k_ErrorHandler_handleSemanticalError(errorHandler,
                listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_VARIABLE_PARAMETER,
                (k_Token_t*)(variableParameter->m_context));
        }
        else {
            symbol = k_Symbol_forConstant(variableParameter, symbolTable->m_currentScope);
            k_Symbol_addModifiers(symbol, KUSH_MODIFIER_VARIABLE_PARAMETER);
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }
    }
}

void k_SymbolDefinitionListener_onExitFunctionDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;

    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

/* functionParameters */

void k_SymbolDefinitionListener_onEnterStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Scope_t* scope = k_Scope_forLocal(listener->m_symbolTable->m_currentScope);
    k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
    k_ASTAnnotations_put(listener->m_scopes, node, scope);
}

void k_SymbolDefinitionListener_onExitStatementSuite(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

void k_SymbolDefinitionListener_onEnterVariableDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_VariableDeclarationContext_t* variableDeclarationContext = (k_VariableDeclarationContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(variableDeclarationContext->m_variableDeclarators);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* variableDeclarator = jtk_ArrayList_getValue(variableDeclarationContext->m_variableDeclarators, i);
        k_VariableDeclaratorContext_t* variableDeclaratorContext = (k_VariableDeclaratorContext_t*)variableDeclarator->m_context;

        k_ASTNode_t* identifier = variableDeclaratorContext->m_identifier;
        const uint8_t* identifierText = k_Token_getText((k_Token_t*)identifier->m_context);

        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
        if (symbol != NULL) {
            k_ErrorHandler_handleSemanticalError(errorHandler,
                listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_VARIABLE,
                (k_Token_t*)identifier->m_context);
        }
        else {
            k_Symbol_t* symbol = k_Symbol_forVariable(identifier,
                listener->m_symbolTable->m_currentScope);
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }
    }
}

void k_SymbolDefinitionListener_onEnterConstantDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ConstantDeclarationContext_t* constantDeclarationContext = (k_ConstantDeclarationContext_t*)node->m_context;

    int32_t size = jtk_ArrayList_getSize(constantDeclarationContext->m_constantDeclarators);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_ASTNode_t* constantDeclarator = jtk_ArrayList_getValue(constantDeclarationContext->m_constantDeclarators, i);
        k_ConstantDeclaratorContext_t* constantDeclaratorContext = (k_ConstantDeclaratorContext_t*)constantDeclarator->m_context;

        k_ASTNode_t* identifier = constantDeclaratorContext->m_identifier;
        const uint8_t* identifierText = k_Token_getText((k_Token_t*)identifier->m_context);

        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
        if (symbol != NULL) {
            k_ErrorHandler_handleSemanticalError(errorHandler,
                listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CONSTANT,
                (k_Token_t*)identifier->m_context);
        }
        else {
            k_Symbol_t* symbol = k_Symbol_forConstant(identifier, listener->m_symbolTable->m_currentScope);
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }
    }
}

void k_SymbolDefinitionListener_onEnterLabelClause(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_LabelClauseContext_t* LabelClauseContext = (k_LabelClauseContext_t*)node->m_context;

    k_ASTNode_t* identifier = LabelClauseContext->m_identifier;
    const uint8_t* identifierText = k_Token_getText((k_Token_t*)identifier->m_context);

    k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
    if (symbol != NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler,
            listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_LABEL,
            (k_Token_t*)identifier->m_context);
    }
    else {
        k_Symbol_t* symbol = k_Symbol_forLabel(identifier,
            listener->m_symbolTable->m_currentScope);
        k_SymbolTable_define(listener->m_symbolTable, symbol);
    }
}

// TODO: Each for statement should receive its own local scope to define
// its item variable.
void k_SymbolDefinitionListener_onEnterForParameters(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ForParameterContext_t* forParametersContext = (k_ForParameterContext_t*)node->m_context;

    if (forParametersContext->m_declaration) {
        k_ASTNode_t* identifier = forParametersContext->m_identifier;
        const uint8_t* identifierText = k_Token_getText((k_Token_t*)identifier->m_context);

        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
        if (symbol != NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler,
            listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_LOOP_PARAMETER,
            (k_Token_t*)identifier->m_context);
        }
        else {
            if (forParametersContext->m_variable) {
                symbol = k_Symbol_forVariable(identifier,
                    listener->m_symbolTable->m_currentScope);
            }
            else {
                symbol = k_Symbol_forConstant(identifier,
                    listener->m_symbolTable->m_currentScope);
            }
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }
    }
}

void k_SymbolDefinitionListener_onEnterTryStatement(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_TryStatementContext_t* context = (k_TryStatementContext_t*)node->m_context;
    k_ASTWalker_walk(astListener, context->m_tryClause);

    int32_t catchClauseCount = jtk_ArrayList_getSize(context->m_catchClauses);
    int32_t i;
    for (i = 0; i < catchClauseCount; i++) {
        k_ASTNode_t* catchClause = (k_ASTNode_t*)jtk_ArrayList_getValue(context->m_catchClauses, i);
        k_CatchClauseContext_t* catchClauseContext = (k_CatchClauseContext_t*)catchClause->m_context;

        /* Each catch clause receives its own local scope to define its
         * catch parameter.
         */
        k_Scope_t* scope = k_Scope_forLocal(listener->m_symbolTable->m_currentScope);
        k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
        k_ASTAnnotations_put(listener->m_scopes, catchClause, scope);

        k_ASTNode_t* identifier = catchClauseContext->m_identifier;
        int32_t identifierSize;
        uint8_t* identifierText = k_ASTNode_toCString(identifier, &identifierSize);

        k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
        if (symbol != NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler,
            listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CATCH_PARAMETER,
            (k_Token_t*)identifier->m_context);
        }
        else {
            symbol = k_Symbol_forVariable(identifier, scope);
            k_SymbolTable_define(listener->m_symbolTable, symbol);
        }

        /* Visit the scopes of the statement suite specified to the
         * catch clause.
         */
        k_ASTWalker_walk(astListener, catchClauseContext->m_statementSuite);

        k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);

        /* Destroy the identifier text. It is not required anymore. */
        jtk_Memory_deallocate(identifierText);
    }

    k_ASTListener_skipChildren(astListener);
}

void k_SymbolDefinitionListener_onEnterClassDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");

    k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    k_Compiler_t* compiler = listener->m_compiler;
    k_ErrorHandler_t* errorHandler = compiler->m_errorHandler;
    k_ClassDeclarationContext_t* classDeclarationContext = (k_ClassDeclarationContext_t*)node->m_context;

    k_ASTNode_t* identifier = classDeclarationContext->m_identifier;
    k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    const uint8_t* identifierText = k_Token_getText(identifierToken);

    k_Symbol_t* classSymbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
    if (classSymbol != NULL) {
        k_ErrorHandler_handleSemanticalError(errorHandler,
            listener, KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CLASS,
            (k_Token_t*)identifier->m_context);
    }
    else {
        k_Scope_t* scope = k_Scope_forClass(listener->m_symbolTable->m_currentScope);

        k_Symbol_t* symbol = k_Symbol_forClass(identifier,
            listener->m_symbolTable->m_currentScope, scope,
            identifierToken->m_text, identifierToken->m_length,
            listener->m_package, listener->m_packageSize);
        k_ClassSymbol_t* classSymbol = &symbol->m_context;
        scope->m_symbol = symbol;

        k_SymbolTable_define(listener->m_symbolTable, symbol);
        k_Compiler_registerSymbol(compiler, classSymbol->m_qualifiedName,
            classSymbol->m_qualifiedNameSize, symbol);

        k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
        k_ASTAnnotations_put(listener->m_scopes, node, scope);

        k_ASTWalker_walk(astListener, classDeclarationContext->m_classSuite);

        k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
    }

    /* If the definition of a class is duplicate, then do not visit the children
     * nodes.
     */
    k_ASTListener_skipChildren(astListener);
}

void k_SymbolDefinitionListener_onExitClassDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {
    jtk_Assert_assertObject(astListener, "The specified AST listener is null.");
    jtk_Assert_assertObject(node, "The specified AST node is null.");
}

void k_SymbolDefinitionListener_onEnterEnumerationDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {

    // k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    // k_EnumerationDeclarationContext_t* enumerationDeclarationContext = (k_EnumerationDeclarationContext_t*)node->m_context;

    // k_ASTNode_t* identifier = enumerationDeclarationContext->m_identifier;
    // k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    // const uint8_t* identifierText = k_Token_getText(identifierToken);

    // k_Symbol_t* symbol = k_SymbolTable_resolve(listener->m_symbolTable, identifierText);
    // if (symbol != NULL) {
    //     k_ErrorHandler_reportError(NULL, "Redeclaration of symbol as enumeration", (k_Token_t*)identifier->m_context);
    // }
    // else {
    //     k_EnumerationScope_t* enumerationScope = k_EnumerationScope_new(listener->m_symbolTable->m_currentScope);

    //     k_EnumerationSymbol_t* enumerationSymbol = k_EnumerationSymbol_new(identifier, listener->m_symbolTable->m_currentScope, k_EnumerationScope_getScope(enumerationScope));
    //     symbol = k_EnumerationSymbol_getSymbol(enumerationSymbol);
    //     k_SymbolTable_define(listener->m_symbolTable, symbol);

    //     enumerationScope->m_enumerationSymbol = symbol;

    //     k_Scope_t* scope = k_EnumerationScope_getScope(enumerationScope);

    //     k_SymbolTable_setCurrentScope(listener->m_symbolTable, scope);
    //     k_ASTAnnotations_put(listener->m_scopes, node, scope);
    // }
}

void k_SymbolDefinitionListener_onExitEnumerationDeclaration(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {

    // k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    // k_EnumerationDeclarationContext_t* enumerationDeclarationContext = (k_EnumerationDeclarationContext_t*)node->m_context;

    // k_SymbolTable_invalidateCurrentScope(listener->m_symbolTable);
}

void k_SymbolDefinitionListener_onEnterEnumerate(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {

    // k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    // k_EnumerationDeclarationContext_t* enumerationDeclarationContext = (k_EnumerationDeclarationContext_t*)node->m_context;

    // k_ASTNode_t* identifier = enumerationDeclarationContext->m_identifier;
    // k_Token_t* identifierToken = (k_Token_t*)identifier->m_context;
    // const uint8_t* identifierText = k_Token_getText(identifierToken);

    // k_Scope_t* currentScope = k_SymbolTable_getCurrentScope(listener->m_symbolTable);
    // if (k_Scope_isEnumerationScope(currentScope)) {
    //     k_Symbol_t* symbol = k_Scope_resolve(currentScope, identifierText);
    //     if (symbol != NULL) {
    //         k_ErrorHandler_reportError(NULL, "Duplicate enumerate", (k_Token_t*)identifier->m_context);
    //     }
    //     else {
    //         k_EnumerateSymbol_t* enumerateSymbol = k_EnumerateSymbol_new(identifier, listener->m_symbolTable->m_currentScope);
    //         symbol = k_EnumerateSymbol_getSymbol(enumerateSymbol);
    //         k_SymbolTable_define(listener->m_symbolTable, symbol);
    //     }
    // }
    // else {
    //     fprintf(stderr, "[internal error] Declaration of enumerate in unsuitable scope.\n");
    // }
}

void k_SymbolDefinitionListener_onExitEnumerate(
    k_ASTListener_t* astListener, k_ASTNode_t* node) {

    // k_SymbolDefinitionListener_t* listener = (k_SymbolDefinitionListener_t*)astListener->m_context;
    // k_EnumerationDeclarationContext_t* enumerationDeclarationContext = (k_EnumerationDeclarationContext_t*)node->m_context;
}

/* When an error occurs in a nested function, it prevents the listener from
 * creating a scope. But then, when exit event invalidates a scope which does
 * not exist. This causes the program to crash.
 * Fix this.
 * Further, a match parameter should be accepted by the invalidate function
 * to prevent such mismatches.
 */

/* When a function declaration is erroneous, it is not defined in the symbol
 * table. But a corresponding scope is openened anyway. Who deletes this scope?
 */

/* TODO: BUG!!!! When an error occurs in a class or enumeration declaration,
   a scope is not opened. But when the declaration context exits the listener
   automatically closes a scope which does not exist! This causes a segmentation
   fault. A possible fix is to use a state variable. But then it will
   get tampered a nested declaration occurs!
*/
