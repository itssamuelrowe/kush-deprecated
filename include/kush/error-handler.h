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
enum ErrorCode {
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

    ERROR_INCOMPATIBLE_OPERAND,
    ERROR_INCOMPATIBLE_OPERAND_TYPES,
    ERROR_COMBINING_EQUALITY_OPERATORS,
    ERROR_UNKNOWN_MODULE,
    ERROR_INVALID_LEFT_OPERAND,
    ERROR_INVALID_FUNCTION_INVOCATION,
    ERROR_INVALID_MEMBER_ACCESS,

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

typedef enum ErrorCode ErrorCode;

/*******************************************************************************
 * Error                                                                       *
 *******************************************************************************/

struct Error {
    ErrorCode code;
    Token* token;
    TokenType expected;
};

typedef struct Error Error;

Error* errorNew(ErrorCode errorCode, Token* token);
Error* errorNewEx(ErrorCode errorCode, Token* token, TokenType expected);

// Destructor

void errorDelete(Error* error);

/*******************************************************************************
 * ErrorHandler                                                                *
 *******************************************************************************/

typedef struct Lexer Lexer;
typedef struct Parser Parser;

/* NOTE: The handlers should not maintain any reference to the origin
 * object that they receive. Because errors may persist beyond the lifetime
 * of an origin object.
 */

typedef void (*OnLexicalErrorFunction)(void* context, Lexer* lexer, Error* error);
typedef void (*OnSyntaxErrorFunction)(void* context, Parser* parser, Error* error, TokenType expected);
typedef void (*OnSemanticErrorFunction)(void* context, void* origin, Error* error);

typedef void (*OnGeneralErrorFunction)(void* context, void* origin, Error* error);

struct ErrorHandler {
    OnLexicalErrorFunction onLexicalError;
    OnSyntaxErrorFunction onSyntaxError;
    OnSemanticErrorFunction onSemanticError;
    OnGeneralErrorFunction onGeneralError;

    jtk_ArrayList_t* errors;
    void* context;
};

typedef struct ErrorHandler ErrorHandler;

ErrorHandler* newErrorHandler();
void deleteErrorHandler(ErrorHandler* handler);
void handleSyntaxError(ErrorHandler* handler,
    Parser* parser, ErrorCode errorCode, Token* token,
    TokenType expected);
void handleLexicalError(ErrorHandler* handler,
    Lexer* lexer, ErrorCode errorCode, Token* token);
void handleSemanticalError(ErrorHandler* handler,
    void* origin, ErrorCode errorCode, Token* token);
void handleGeneralError(ErrorHandler* handler,
    void* origin, ErrorCode errorCode);

#endif /* KUSH_COMPILER_SUPPORT_ERROR_HANDLER_H */
