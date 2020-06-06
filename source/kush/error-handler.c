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
    k_ErrorHandler_t* handler = jtallocate(k_ErrorHandler_t, 1);
    handler->handleLexicalError = NULL;
    handler->handleSyntacticalError = NULL;
    handler->handleSemanticalError = NULL;
    handler->onLexicalError = NULL;
    handler->onSyntacticalError = NULL;
    handler->onSemanticalError = NULL;
    handler->errors = jtk_ArrayList_new();
    handler->active = true;
    handler->context = NULL;

    return handler;
}

// Destructor

void k_ErrorHandler_delete(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    int32_t errorCount = jtk_ArrayList_getSize(handler->errors);
    int32_t i;
    for (i = 0; i < errorCount; i++) {
        k_Error_t* error = (k_Error_t*)jtk_ArrayList_getValue(handler->errors, i);
        k_Error_delete(error);
    }
    jtk_ArrayList_delete(handler->errors);
    jtdeallocate(handler);
}

// Active

void k_ErrorHandler_setActive(k_ErrorHandler_t* handler, bool active) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->active = active;
}

bool k_ErrorHandler_isActive(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->active;
}

// Syntactical Error

void k_ErrorHandler_setOnSyntacticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSyntacticalErrorFunction_t onSyntacticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onSyntacticalError = onSyntacticalError;
}

k_ErrorHandler_OnSyntacticalErrorFunction_t k_ErrorHandler_getOnSyntacticalError(
    k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onSemanticalError;
}

void k_ErrorHandler_handleSyntacticalError(k_ErrorHandler_t* handler,
    k_Parser_t* parser, k_ErrorCode_t errorCode, k_Token_t* token,
    k_TokenType_t expected) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_newEx(errorCode, token, expected);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleSyntacticalError != NULL) {
        handler->handleSyntacticalError(handler->context, parser, error);
    }

    if (handler->active && (handler->onSyntacticalError != NULL)) {
        handler->onSyntacticalError(handler->context, parser, error, expected);
    }
}

// Lexical Error

void k_ErrorHandler_setOnLexicalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnLexicalErrorFunction_t onLexicalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onLexicalError = onLexicalError;
}

k_ErrorHandler_OnLexicalErrorFunction_t k_ErrorHandler_getOnLexicalError(
    k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onLexicalError;
}

void k_ErrorHandler_handleLexicalError(k_ErrorHandler_t* handler,
    k_Lexer_t* lexer, k_ErrorCode_t errorCode, k_Token_t* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, token);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleLexicalError != NULL) {
        handler->handleLexicalError(handler->context, lexer, error);
    }

    if (handler->active && (handler->onLexicalError != NULL)) {
        handler->onLexicalError(handler->context, lexer, error);
    }
}

// Semantic Error

void k_ErrorHandler_setOnSemanticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSemanticalErrorFunction_t onSemanticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onSemanticalError = onSemanticalError;
}

k_ErrorHandler_OnSemanticalErrorFunction_t k_ErrorHandler_getOnSemanticalError(
    k_ErrorHandler_t* handler){
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onSemanticalError;
}

void k_ErrorHandler_handleSemanticalError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode, k_Token_t* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, token);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleSemanticalError != NULL) {
        handler->handleSemanticalError(handler->context, origin, error);
    }

    if (handler->active && (handler->onSemanticalError != NULL)) {
        handler->onSemanticalError(handler->context, origin, error);
    }
}

void k_ErrorHandler_handleGeneralError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    k_Error_t* error = k_Error_new(errorCode, NULL);
    jtk_ArrayList_add(handler->errors, error);

    // if (handler->handleGeneralError != NULL) {
    //     handler->handleGeneralError(handler->context, origin, error);
    // }

    if (handler->active && (handler->onGeneralError != NULL)) {
        handler->onGeneralError(handler->context, origin, error);
    }
}

// Errors

jtk_ArrayList_t* k_ErrorHandler_getErrors(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->errors;
}

int32_t k_ErrorHandler_getErrorCount(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return jtk_ArrayList_getSize(handler->errors);
}

bool k_ErrorHandler_hasErrors(k_ErrorHandler_t* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return !jtk_ArrayList_isEmpty(handler->errors);
}