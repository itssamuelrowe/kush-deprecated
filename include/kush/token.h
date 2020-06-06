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

// Wedensday, October 18, 2017

#ifndef KUSH_COMPILER_LEXER_TOKEN_H
#define KUSH_COMPILER_LEXER_TOKEN_H

#include <kush/Configuration.h>
#include <kush/lexer/TokenType.h>
#include <kush/lexer/TokenChannel.h>

/*******************************************************************************
 * TokenType                                                                   *
 *******************************************************************************/

/**
 * @class TokenType
 * @ingroup k_compiler_lexer
 * @author Samuel Rowe
 * @since Kush 0.1
 */
enum k_TokenType_t {
    KUSH_TOKEN_UNKNOWN,

    KUSH_TOKEN_INDENTATION,
    KUSH_TOKEN_DEDENTATION,

    KUSH_TOKEN_WHITESPACE,
    KUSH_TOKEN_NEWLINE,
    KUSH_TOKEN_END_OF_STREAM,

    KUSH_TOKEN_EXCLAMATION_MARK_EQUAL,
    KUSH_TOKEN_EXCLAMATION_MARK,

    KUSH_TOKEN_AT,

    KUSH_TOKEN_HASH,

    KUSH_TOKEN_MODULUS_EQUAL,
    KUSH_TOKEN_MODULUS,

    KUSH_TOKEN_AMPERSAND_2,
    KUSH_TOKEN_AMPERSAND_EQUAL,
    KUSH_TOKEN_AMPERSAND,

    KUSH_TOKEN_LEFT_PARENTHESIS,
    KUSH_TOKEN_RIGHT_PARENTHESIS,

    KUSH_TOKEN_ASTERISK_2_EQUAL,
    KUSH_TOKEN_ASTERISK_2,
    KUSH_TOKEN_ASTERISK_EQUAL,
    KUSH_TOKEN_ASTERISK,

    KUSH_TOKEN_PLUS_2,
    KUSH_TOKEN_PLUS_EQUAL,
    KUSH_TOKEN_PLUS,

    KUSH_TOKEN_COMMA,

    KUSH_TOKEN_DASH_2,
    KUSH_TOKEN_ARROW,
    KUSH_TOKEN_DASH_EQUAL,
    KUSH_TOKEN_DASH,

    KUSH_TOKEN_ELLIPSIS,
    KUSH_TOKEN_DOT_2,
    KUSH_TOKEN_DOT,

    KUSH_TOKEN_SINGLE_LINE_COMMENT,
    KUSH_TOKEN_MULTI_LINE_COMMENT,
    KUSH_TOKEN_FORWARD_SLASH_EQUAL,
    KUSH_TOKEN_FORWARD_SLASH,

    KUSH_TOKEN_COLON_2,
    KUSH_TOKEN_COLON,

    KUSH_TOKEN_SEMICOLON,

    KUSH_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL,
    KUSH_TOKEN_LEFT_ANGLE_BRACKET_2,
    KUSH_TOKEN_LEFT_ANGLE_BRACKET_EQUAL,
    KUSH_TOKEN_LEFT_ANGLE_BRACKET,

    KUSH_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL,
    KUSH_TOKEN_RIGHT_ANGLE_BRACKET_3,
    KUSH_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL,
    KUSH_TOKEN_RIGHT_ANGLE_BRACKET_2,
    KUSH_TOKEN_RIGHT_ANGLE_BRACKET_EQUAL,
    KUSH_TOKEN_RIGHT_ANGLE_BRACKET,

    KUSH_TOKEN_EQUAL_2,
    KUSH_TOKEN_EQUAL,

    KUSH_TOKEN_HOOK,

    KUSH_TOKEN_LEFT_BRACE,
    KUSH_TOKEN_RIGHT_BRACE,

    KUSH_TOKEN_LEFT_SQUARE_BRACKET,
    KUSH_TOKEN_RIGHT_SQUARE_BRACKET,

    KUSH_TOKEN_CARET_EQUAL,
    KUSH_TOKEN_CARET,

    KUSH_TOKEN_VERTICAL_BAR_2,
    KUSH_TOKEN_VERTICAL_BAR_EQUAL,
    KUSH_TOKEN_VERTICAL_BAR,

    KUSH_TOKEN_TILDE_EQUAL,
    KUSH_TOKEN_TILDE,

    KUSH_TOKEN_IDENTIFIER,

    /* Keywords */

    KUSH_TOKEN_KEYWORD_BOOLEAN,
    KUSH_TOKEN_KEYWORD_BREAK,
    KUSH_TOKEN_KEYWORD_CATCH,
    KUSH_TOKEN_KEYWORD_ELSE,
    KUSH_TOKEN_KEYWORD_F32,
    KUSH_TOKEN_KEYWORD_F64,
    KUSH_TOKEN_KEYWORD_FALSE,
    KUSH_TOKEN_KEYWORD_FINALLY,
    KUSH_TOKEN_KEYWORD_FOR,
    KUSH_TOKEN_KEYWORD_I16,
    KUSH_TOKEN_KEYWORD_I32,
    KUSH_TOKEN_KEYWORD_I64,
    KUSH_TOKEN_KEYWORD_I8,
    KUSH_TOKEN_KEYWORD_IF,
    KUSH_TOKEN_KEYWORD_IMPORT,
    KUSH_TOKEN_KEYWORD_LET,
    KUSH_TOKEN_KEYWORD_NATIVE,
    KUSH_TOKEN_KEYWORD_NEW,
    KUSH_TOKEN_KEYWORD_NULL,
    KUSH_TOKEN_KEYWORD_RETURN,
    KUSH_TOKEN_KEYWORD_STRUCT,
    KUSH_TOKEN_KEYWORD_THIS,
    KUSH_TOKEN_KEYWORD_THROW,
    KUSH_TOKEN_KEYWORD_TRUE,
    KUSH_TOKEN_KEYWORD_TRY,
    KUSH_TOKEN_KEYWORD_UI16,
    KUSH_TOKEN_KEYWORD_UI32,
    KUSH_TOKEN_KEYWORD_UI64,
    KUSH_TOKEN_KEYWORD_UI8,
    KUSH_TOKEN_KEYWORD_VAR,
    KUSH_TOKEN_KEYWORD_VOID,
    KUSH_TOKEN_KEYWORD_WITH,
    KUSH_TOKEN_KEYWORD_WHILE,

    KUSH_TOKEN_INTEGER_LITERAL,
    KUSH_TOKEN_STRING_LITERAL,
    KUSH_TOKEN_FLOATING_POINT_LITERAL
};

/**
 * @memberof TokenType
 */
typedef enum k_TokenType_t k_TokenType_t;

/*******************************************************************************
 * TokenChannel                                                                *
 *******************************************************************************/

/**
 * @class TokenChannel
 * @ingroup k_compiler_lexer
 * @author Samuel Rowe
 * @since Kush 0.1
 */
enum k_TokenChannel_t {
    KUSH_TOKEN_CHANNEL_DEFAULT,
    KUSH_TOKEN_CHANNEL_HIDDEN,
};

/**
 * @memberof TokenChannel
 */
typedef enum k_TokenChannel_t k_TokenChannel_t;

/*******************************************************************************
 * Token                                                                       *
 *******************************************************************************/

/**
 * A token represents the smallest entity that appears
 * in a source code. Each token has two primary attributes:
 * a token type (symbol category) and the text associated
 * with it.
 *
 * @author Samuel Rowe
 * @since  KUSH 1.0
 */
struct k_Token_t {
    k_TokenChannel_t channel;
    k_TokenType_t type;
    uint8_t* text;
    int32_t length;
    int32_t startIndex;
    int32_t stopIndex;
    int32_t startLine;
    int32_t stopLine;
    int32_t startColumn;
    int32_t stopColumn;
    int32_t index;
    const char* file;
};

/**
 * @memberof Token
 */
typedef struct k_Token_t k_Token_t;

// Constructor

/**
 * @memberof Token
 */
k_Token_t* k_Token_new(k_TokenChannel_t channel, k_TokenType_t type,
    const uint8_t* text, int32_t length, int32_t startIndex, int32_t stopIndex,
    int32_t startLine, int32_t stopLine, int32_t startColumn, int32_t stopColumn,
    const char* file);

// Destructor

/**
 * @memberof Token
 */
void k_Token_delete(k_Token_t* token);

// Channel

/**
 * @memberof Token
 */
k_TokenChannel_t k_Token_getChannel(k_Token_t* token);

/**
 * @memberof Token
 */
void k_Token_setChannel(k_Token_t* token, k_TokenChannel_t channel);

// Type

/**
 * @memberof Token
 */
k_TokenType_t k_Token_getType(k_Token_t* token);

/**
 * @memberof Token
 */
void k_Token_setType(k_Token_t* token, k_TokenType_t type);

// Text

/**
 * @memberof Token
 */
void k_Token_setText(k_Token_t* token, const uint8_t* text, int32_t length);

/**
 * @memberof Token
 */
const uint8_t* k_Token_getText(k_Token_t* token);

// Length

/**
 * @memberof Token
 */
int32_t k_Token_getLength(k_Token_t* token);

// Start Index

/**
 * @memberof Token
 */
void k_Token_setStartIndex(k_Token_t* token, int32_t startIndex);

/**
 * @memberof Token
 */
void k_Token_setStopIndex(k_Token_t* token, int32_t stopIndex);

// Start Line

/**
 * @memberof Token
 */
void k_Token_setStartLine(k_Token_t* token, int32_t startLine);

/**
 * @memberof Token
 */
int32_t k_Token_getStartLine(k_Token_t* token);

// Stop Line

/**
 * @memberof Token
 */
void k_Token_setStopLine(k_Token_t* token, int32_t stopLine);

/**
 * @memberof Token
 */
int32_t k_Token_getStopLine(k_Token_t* token);

// Start Column

/**
 * @memberof Token
 */
void k_Token_setStartColumn(k_Token_t* token, int32_t startColumn);

/**
 * @memberof Token
 */
int32_t k_Token_getStartColumn(k_Token_t* token);

// Stop Column

/**
 * @memberof Token
 */
void k_Token_setStopColumn(k_Token_t* token, int32_t stopColumn);

/**
 * @memberof Token
 */
int32_t k_Token_getStopColumn(k_Token_t* token);

// Index

/**
 * @memberof Token
 */
int32_t k_Token_getIndex(k_Token_t* token);

/**
 * @memberof Token
 */
void k_Token_setIndex(k_Token_t* token, int32_t index);

#endif /* KUSH_COMPILER_LEXER_TOKEN_H */
