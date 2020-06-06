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

// Monday, March 12, 2018

#ifndef KUSH_COMPILER_SUPPORT_ERROR_HANDLER_H
#define KUSH_COMPILER_SUPPORT_ERROR_HANDLER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/support/Error.h>
#include <kush/support/ErrorCode.h>

/*******************************************************************************
 * ErrorHandler                                                                *
 *******************************************************************************/

struct k_Lexer_t;
struct k_Parser_t;
typedef struct k_Lexer_t k_Lexer_t;
typedef struct k_Parser_t k_Parser_t;

/* NOTE: The handlers should not maintain any reference to the origin
 * object that they receive. Because errors may persist beyond the lifetime
 * of an origin object.
 */

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_HandleLexicalErrorFunction_t)(void* context, k_Lexer_t* lexer, k_Error_t* error);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_HandleSyntacticalErrorFunction_t)(void* context, k_Parser_t* parser, k_Error_t* error);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_HandleSemanticalErrorFunction_t)(void* context, void* origin, k_Error_t* error);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_OnLexicalErrorFunction_t)(void* context, k_Lexer_t* lexer, k_Error_t* error);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_OnSyntacticalErrorFunction_t)(void* context, k_Parser_t* parser, k_Error_t* error, k_TokenType_t expected);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_OnSemanticalErrorFunction_t)(void* context, void* origin, k_Error_t* error);

/**
 * @memberof ErrorHandler
 */
typedef void (*k_ErrorHandler_OnGeneralErrorFunction_t)(void* context, void* origin, k_Error_t* error);

/**
 * @class ErrorHandler
 * @author Samuel Rowe
 * @ingroup k_compiler
 * @since Kush 0.1
 */
struct k_ErrorHandler_t {
    k_ErrorHandler_HandleLexicalErrorFunction_t m_handleLexicalError;
    k_ErrorHandler_HandleSyntacticalErrorFunction_t m_handleSyntacticalError;
    k_ErrorHandler_HandleSemanticalErrorFunction_t m_handleSemanticalError;

    k_ErrorHandler_OnLexicalErrorFunction_t m_onLexicalError;
    k_ErrorHandler_OnSyntacticalErrorFunction_t m_onSyntacticalError;
    k_ErrorHandler_OnSemanticalErrorFunction_t m_onSemanticalError;
    k_ErrorHandler_OnGeneralErrorFunction_t m_onGeneralError;

    jtk_ArrayList_t* m_errors;
    bool m_active;
    void* m_context;
};

/**
 * @memberof ErrorHandler
 */
typedef struct k_ErrorHandler_t k_ErrorHandler_t;

// Constructor

/**
 * @memberof ErrorHandler
 */
k_ErrorHandler_t* k_ErrorHandler_new();

// Destructor

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_delete(k_ErrorHandler_t* handler);

// Active

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_setActive(k_ErrorHandler_t* handler, bool active);

/**
 * @memberof ErrorHandler
 */
bool k_ErrorHandler_isActive(k_ErrorHandler_t* handler);

// Syntactical Error

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_setOnSyntacticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSyntacticalErrorFunction_t onSyntacticalError);

/**
 * @memberof ErrorHandler
 */
k_ErrorHandler_OnSyntacticalErrorFunction_t k_ErrorHandler_getOnSyntacticalError(
    k_ErrorHandler_t* handler);

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_handleSyntacticalError(k_ErrorHandler_t* handler,
    k_Parser_t* parser, k_ErrorCode_t errorCode, k_Token_t* token,
    k_TokenType_t expected);

// Lexical Error

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_setOnLexicalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnLexicalErrorFunction_t onLexicalError);

/**
 * @memberof ErrorHandler
 */
k_ErrorHandler_OnLexicalErrorFunction_t k_ErrorHandler_getOnLexicalError(
    k_ErrorHandler_t* handler);

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_handleLexicalError(k_ErrorHandler_t* handler,
    k_Lexer_t* lexer, k_ErrorCode_t errorCode, k_Token_t* token);

// Semantic Error

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_setOnSemanticalError(k_ErrorHandler_t* handler,
    k_ErrorHandler_OnSemanticalErrorFunction_t onSemanticalError);

/**
 * @memberof ErrorHandler
 */
k_ErrorHandler_OnSemanticalErrorFunction_t k_ErrorHandler_getOnSemanticalError(
    k_ErrorHandler_t* handler);

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_handleSemanticalError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode, k_Token_t* token);

/**
 * @memberof ErrorHandler
 */
void k_ErrorHandler_handleGeneralError(k_ErrorHandler_t* handler,
    void* origin, k_ErrorCode_t errorCode);

// Errors

/**
 * @memberof ErrorHandler
 */
jtk_ArrayList_t* k_ErrorHandler_getErrors(k_ErrorHandler_t* handler);

int32_t k_ErrorHandler_getErrorCount(k_ErrorHandler_t* handler);

bool k_ErrorHandler_hasErrors(k_ErrorHandler_t* handler);

#endif /* KUSH_COMPILER_SUPPORT_ERROR_HANDLER_H */
