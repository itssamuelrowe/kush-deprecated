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

#include <kush/error-handler.h>

/*******************************************************************************
 * Error                                                                       *
 *******************************************************************************/

// Constructor

Error* errorNew(ErrorCode code, Token* token) {
    return errorNewEx(code, token, TOKEN_UNKNOWN);
}

Error* errorNewEx(ErrorCode code, Token* token,
    TokenType expected) {
    Error* error = allocate(Error, 1);
    error->code = code;
    error->token = token;
    error->expected = expected;

    return error;
}

// Destructor

void errorDelete(Error* error) {
    jtk_Assert_assertObject(error, "The specified error is null.");

    deallocate(error);
}

/*******************************************************************************
 * ErrorHandler                                                                *
 *******************************************************************************/

// Constructor

ErrorHandler* new() {
    ErrorHandler* handler = allocate(ErrorHandler, 1);
    handler->onLexicalError = NULL;
    handler->onSyntaxError = NULL;
    handler->onSemanticError = NULL;
    handler->errors = jtk_ArrayList_new();
    handler->context = NULL;

    return handler;
}

// Destructor

void delete(ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    int32_t errorCount = jtk_ArrayList_getSize(handler->errors);
    int32_t i;
    for (i = 0; i < errorCount; i++) {
        Error* error = (Error*)jtk_ArrayList_getValue(handler->errors, i);
        errorDelete(error);
    }
    jtk_ArrayList_delete(handler->errors);
    deallocate(handler);
}

// Active

void setActive(ErrorHandler* handler, bool active) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->active = active;
}

bool isActive(ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->active;
}

// Syntactical Error

void setOnSyntacticalError(ErrorHandler* handler,
    OnSyntaxErrorFunction onSyntacticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onSyntacticalError = onSyntacticalError;
}

OnSyntaxErrorFunction getOnSyntacticalError(
    ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onSemanticalError;
}

void handleSyntacticalError(ErrorHandler* handler,
    Parser* parser, ErrorCode errorCode, Token* token,
    TokenType expected) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    Error* error = errorNewEx(errorCode, token, expected);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleSyntacticalError != NULL) {
        handler->handleSyntacticalError(handler->context, parser, error);
    }

    if (handler->active && (handler->onSyntacticalError != NULL)) {
        handler->onSyntacticalError(handler->context, parser, error, expected);
    }
}

// Lexical Error

void setOnLexicalError(ErrorHandler* handler,
    OnLexicalErrorFunction onLexicalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onLexicalError = onLexicalError;
}

OnLexicalErrorFunction getOnLexicalError(
    ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onLexicalError;
}

void handleLexicalError(ErrorHandler* handler,
    Lexer* lexer, ErrorCode errorCode, Token* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    Error* error = errorNew(errorCode, token);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleLexicalError != NULL) {
        handler->handleLexicalError(handler->context, lexer, error);
    }

    if (handler->active && (handler->onLexicalError != NULL)) {
        handler->onLexicalError(handler->context, lexer, error);
    }
}

// Semantic Error

void setOnSemanticalError(ErrorHandler* handler,
    OnSemanticErrorFunction onSemanticalError) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    handler->onSemanticalError = onSemanticalError;
}

OnSemanticErrorFunction getOnSemanticalError(
    ErrorHandler* handler){
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->onSemanticalError;
}

void handleSemanticalError(ErrorHandler* handler,
    void* origin, ErrorCode errorCode, Token* token) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    Error* error = errorNew(errorCode, token);
    jtk_ArrayList_add(handler->errors, error);

    if (handler->handleSemanticalError != NULL) {
        handler->handleSemanticalError(handler->context, origin, error);
    }

    if (handler->active && (handler->onSemanticalError != NULL)) {
        handler->onSemanticalError(handler->context, origin, error);
    }
}

void handleGeneralError(ErrorHandler* handler,
    void* origin, ErrorCode errorCode) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    Error* error = errorNew(errorCode, NULL);
    jtk_ArrayList_add(handler->errors, error);

    // if (handler->handleGeneralError != NULL) {
    //     handler->handleGeneralError(handler->context, origin, error);
    // }

    if (handler->active && (handler->onGeneralError != NULL)) {
        handler->onGeneralError(handler->context, origin, error);
    }
}

// Errors

jtk_ArrayList_t* getErrors(ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return handler->errors;
}

int32_t getErrorCount(ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return jtk_ArrayList_getSize(handler->errors);
}

bool hasErrors(ErrorHandler* handler) {
    jtk_Assert_assertObject(handler, "The specified error handler is null.");

    return !jtk_ArrayList_isEmpty(handler->errors);
}