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

#include <kush/configuration.h>
#include <kush/token.h>

/*******************************************************************************
 * ErrorCode                                                                   *
 *******************************************************************************/

/**
 * @class ErrorCode
 * @ingroup k_compiler_support
 * @author Samuel Rowe
 * @since Kush 0.1
 */
enum k_ErrorCode_t {
    KUSH_ERROR_CODE_NONE = 0,

    // Lexcial Errors

    KUSH_ERROR_CODE_UNTERMINATED_STRING_LITERAL,
    KUSH_ERROR_CODE_UNTERMINATED_MULTI_LINE_COMMENT,
    KUSH_ERROR_CODE_MALFORMED_UNICODE_CHARACTER_SEQUENCE,
    KUSH_ERROR_CODE_INVALID_ESCAPE_SEQUENCE,
    KUSH_ERROR_CODE_UNKNOWN_CHARACTER,
    KUSH_ERROR_CODE_INVALID_INTEGER_LITERAL_PREFIX,
    KUSH_ERROR_CODE_EXPECTED_DIGIT_AFTER_UNDERSCORE,

    // Syntactical Errors

    KUSH_ERROR_CODE_UNEXPECTED_TOKEN,
    KUSH_ERROR_CODE_TRY_STATEMENT_EXPECTS_CATCH_OR_FINALLY,

    // Semantical Errors

    KUSH_ERROR_CODE_UNDECLARED_CLASS,
    KUSH_ERROR_CODE_INVALID_LVALUE,
    KUSH_ERROR_CODE_INSTANTIATION_OF_NON_CLASS_SYMBOL,
    KUSH_ERROR_CODE_NO_SUITABLE_CONSTRUCTOR,
    KUSH_ERROR_CODE_UNDECLARED_IDENTIFIER,
    KUSH_ERROR_CODE_VARIABLE_TREATED_AS_FUNCTION,
    KUSH_ERROR_CODE_STATIC_INITIALIZER_WITH_PARAMETERS,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_FUNCTION,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_PARAMETER,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_VARIABLE_PARAMETER,
    KUSH_ERROR_CODE_MULTIPLE_FUNCTION_OVERLOADS_WITH_VARIABLE_PARAMETER,
    KUSH_ERROR_CODE_DUPLICATE_FUNCTION_OVERLOAD,
    KUSH_ERROR_CODE_FUNCTION_DECLARATION_EXCEEDS_PARAMETER_THRESHOLD,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_VARIABLE,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CONSTANT,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_LABEL,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_LOOP_PARAMETER,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CATCH_PARAMETER,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_AS_CLASS,
    KUSH_ERROR_CODE_UNKNOWN_CLASS,
    KUSH_ERROR_CODE_REDECLARATION_OF_SYMBOL_PREVIOUSLY_IMPORTED,
    KUSH_ERROR_CODE_CANNOT_DECLARE_MULTIPLE_TYPES_OF_COMPONENT,
    KUSH_ERROR_CODE_FUNCTION_DECLARATION_CAUSES_ANOTHER_FUNCTION_TO_EXCEED_PARAMETER_THRESHOLD,

    // General Errors

    KUSH_ERROR_CODE_CORRUPTED_BINARY_ENTITY,
    KUSH_ERROR_CODE_INVALID_FEB_VERSION,

    KUSH_ERROR_CODE_COUNT
};

typedef enum k_ErrorCode_t k_ErrorCode_t;

/*******************************************************************************
 * Error                                                                       *
 *******************************************************************************/

/**
 * @class Error
 * @ingroup k_compiler_support
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Error_t {
    k_ErrorCode_t code;
    Token* token;
    TokenType expected;
};

/**
 * @memberof Error
 */
typedef struct k_Error_t k_Error_t;

// Constructor

k_Error_t* k_Error_new(k_ErrorCode_t errorCode, Token* token);
k_Error_t* k_Error_newEx(k_ErrorCode_t errorCode, Token* token, TokenType expected);

// Destructor

void k_Error_delete(k_Error_t* error);

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
typedef void (*k_ErrorHandler_OnSyntacticalErrorFunction_t)(void* context, k_Parser_t* parser, k_Error_t* error, TokenType expected);

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
    k_ErrorHandler_HandleLexicalErrorFunction_t handleLexicalError;
    k_ErrorHandler_HandleSyntacticalErrorFunction_t handleSyntacticalError;
    k_ErrorHandler_HandleSemanticalErrorFunction_t handleSemanticalError;

    k_ErrorHandler_OnLexicalErrorFunction_t onLexicalError;
    k_ErrorHandler_OnSyntacticalErrorFunction_t onSyntacticalError;
    k_ErrorHandler_OnSemanticalErrorFunction_t onSemanticalError;
    k_ErrorHandler_OnGeneralErrorFunction_t onGeneralError;

    jtk_ArrayList_t* errors;
    bool active;
    void* context;
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
    k_Parser_t* parser, k_ErrorCode_t errorCode, Token* token,
    TokenType expected);

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
    k_Lexer_t* lexer, k_ErrorCode_t errorCode, Token* token);

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
    void* origin, k_ErrorCode_t errorCode, Token* token);

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
