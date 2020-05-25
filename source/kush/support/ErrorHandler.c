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

// Monday, March 11, 2018

#include <kush/support/ErrorHandler.h>

/*******************************************************************************
 * ErrorHandler                                                                *
 *******************************************************************************/

// Constructor

k_ErrorHandler_t* k_ErrorHandler_new() {
    k_ErrorHandler_t* handler = jtk_Memory_allocate(k_ErrorHandler_t, 1);
    handler->m_handleLexicalError = NULL;
    handler->m_handleSyntacticalError = NULL;
    handler->m_handleSemanticalError = NULL;
    handler->m_onLexicalError = NULL;
    handler->m_onSyntacticalError = NULL;
    handler->m_onSemanticalError = NULL;
    handler->m_errors = jtk_ArrayList_new();
    handler->m_active = true;
    handler->m_context = NULL;

    return handler;
}

// Destructor

void k_ErrorHandler_delete(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    int32_t errorCount = jtk_ArrayList_getSize(handler->m_errors);
    int32_t i;
    for (i = 0; i < errorCount; i++) {
        k_Error_t* error = (k_Error_t*)jtk_ArrayList_getValue(handler->m_errors, i);
        k_Error_delete(error);
    }
    jtk_ArrayList_delete(handler->m_errors);
    jtk_Memory_deallocate(handler);
}

// Active

void k_ErrorHandler_setActive(k_ErrorHandler_t* handler, bool active) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->m_active = active;
}

bool k_ErrorHandler_isActive(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->m_active;
}

// Syntactical Error

void k_ErrorHandler_setOnSyntacticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSyntacticalErrorFunction_t onSyntacticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->m_onSyntacticalError = onSyntacticalError;
}

k_ErrorHandler_OnSyntacticalErrorFunction_t k_ErrorHandler_getOnSyntacticalError(
    k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->m_onSemanticalError;
}

void k_ErrorHandler_handleSyntacticalError(k_ErrorHandler_t* handler,
    k_Parser_t* parser, k_ErrorCode_t errorCode, k_Token_t* token,
    k_TokenType_t expected) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_newEx(errorCode, token, expected);
    jtk_ArrayList_add(handler->m_errors, error);

    if (handler->m_handleSyntacticalError != NULL) {
        handler->m_handleSyntacticalError(handler->m_context, parser, error);
    }

    if (handler->m_active && (handler->m_onSyntacticalError != NULL)) {
        handler->m_onSyntacticalError(handler->m_context, parser, error, expected);
    }
}

// Lexical Error

void k_ErrorHandler_setOnLexicalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnLexicalErrorFunction_t onLexicalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->m_onLexicalError = onLexicalError;
}

k_ErrorHandler_OnLexicalErrorFunction_t k_ErrorHandler_getOnLexicalError(
    k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->m_onLexicalError;
}

void k_ErrorHandler_handleLexicalError(k_ErrorHandler_t* handler,
    k_Lexer_t* lexer, k_ErrorCode_t errorCode, k_Token_t* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, token);
    jtk_ArrayList_add(handler->m_errors, error);

    if (handler->m_handleLexicalError != NULL) {
        handler->m_handleLexicalError(handler->m_context, lexer, error);
    }

    if (handler->m_active && (handler->m_onLexicalError != NULL)) {
        handler->m_onLexicalError(handler->m_context, lexer, error);
    }
}

// Semantic Error

void k_ErrorHandler_setOnSemanticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSemanticalErrorFunction_t onSemanticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->m_onSemanticalError = onSemanticalError;
}

k_ErrorHandler_OnSemanticalErrorFunction_t k_ErrorHandler_getOnSemanticalError(
    k_ErrorHandler_t* handler){
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->m_onSemanticalError;
}

void k_ErrorHandler_handleSemanticalError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode, k_Token_t* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, token);
    jtk_ArrayList_add(handler->m_errors, error);

    if (handler->m_handleSemanticalError != NULL) {
        handler->m_handleSemanticalError(handler->m_context, origin, error);
    }

    if (handler->m_active && (handler->m_onSemanticalError != NULL)) {
        handler->m_onSemanticalError(handler->m_context, origin, error);
    }
}

void k_ErrorHandler_handleGeneralError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, NULL);
    jtk_ArrayList_add(handler->m_errors, error);

    // if (handler->m_handleGeneralError != NULL) {
    //     handler->m_handleGeneralError(handler->m_context, origin, error);
    // }

    if (handler->m_active && (handler->m_onGeneralError != NULL)) {
        handler->m_onGeneralError(handler->m_context, origin, error);
    }
}

// Errors

jtk_ArrayList_t* k_ErrorHandler_getErrors(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->m_errors;
}

int32_t k_ErrorHandler_getErrorCount(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return jtk_ArrayList_getSize(handler->m_errors);
}

bool k_ErrorHandler_hasErrors(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return !jtk_ArrayList_isEmpty(handler->m_errors);
}