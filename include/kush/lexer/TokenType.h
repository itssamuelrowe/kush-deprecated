/*
 * Copyright 2017-2020 Samuel Rowe
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

// Saturday, November 04, 2017

#ifndef COM_ONECUBE_ZEN_COMPILER_LEXER_TOKEN_TYPE_H
#define COM_ONECUBE_ZEN_COMPILER_LEXER_TOKEN_TYPE_H

#include <com/onecube/zen/Configuration.h>

/*******************************************************************************
 * TokenType                                                                   *
 *******************************************************************************/

/**
 * @class TokenType
 * @ingroup zen_compiler_lexer
 * @author Samuel Rowe
 * @since zen 1.0
 */
enum zen_TokenType_t {
    ZEN_TOKEN_UNKNOWN,

    ZEN_TOKEN_INDENTATION,
    ZEN_TOKEN_DEDENTATION,

    ZEN_TOKEN_WHITESPACE,
    ZEN_TOKEN_NEWLINE,
    ZEN_TOKEN_END_OF_STREAM,

    ZEN_TOKEN_EXCLAMATION_MARK_EQUAL,
    ZEN_TOKEN_EXCLAMATION_MARK,

    ZEN_TOKEN_AT,

    ZEN_TOKEN_HASH,

    ZEN_TOKEN_MODULUS_EQUAL,
    ZEN_TOKEN_MODULUS,

    ZEN_TOKEN_AMPERSAND_2,
    ZEN_TOKEN_AMPERSAND_EQUAL,
    ZEN_TOKEN_AMPERSAND,

    ZEN_TOKEN_LEFT_PARENTHESIS,
    ZEN_TOKEN_RIGHT_PARENTHESIS,

    ZEN_TOKEN_ASTERISK_2_EQUAL,
    ZEN_TOKEN_ASTERISK_2,
    ZEN_TOKEN_ASTERISK_EQUAL,
    ZEN_TOKEN_ASTERISK,

    ZEN_TOKEN_PLUS_2,
    ZEN_TOKEN_PLUS_EQUAL,
    ZEN_TOKEN_PLUS,

    ZEN_TOKEN_COMMA,

    ZEN_TOKEN_DASH_2,
    ZEN_TOKEN_ARROW,
    ZEN_TOKEN_DASH_EQUAL,
    ZEN_TOKEN_DASH,

    ZEN_TOKEN_ELLIPSIS,
    ZEN_TOKEN_DOT_2,
    ZEN_TOKEN_DOT,

    ZEN_TOKEN_SINGLE_LINE_COMMENT,
    ZEN_TOKEN_MULTI_LINE_COMMENT,
    ZEN_TOKEN_FORWARD_SLASH_EQUAL,
    ZEN_TOKEN_FORWARD_SLASH,

    ZEN_TOKEN_COLON_2,
    ZEN_TOKEN_COLON,

    ZEN_TOKEN_SEMICOLON,

    ZEN_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL,
    ZEN_TOKEN_LEFT_ANGLE_BRACKET_2,
    ZEN_TOKEN_LEFT_ANGLE_BRACKET_EQUAL,
    ZEN_TOKEN_LEFT_ANGLE_BRACKET,

    ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL,
    ZEN_TOKEN_RIGHT_ANGLE_BRACKET_3,
    ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL,
    ZEN_TOKEN_RIGHT_ANGLE_BRACKET_2,
    ZEN_TOKEN_RIGHT_ANGLE_BRACKET_EQUAL,
    ZEN_TOKEN_RIGHT_ANGLE_BRACKET,

    ZEN_TOKEN_EQUAL_2,
    ZEN_TOKEN_EQUAL,

    ZEN_TOKEN_HOOK,

    ZEN_TOKEN_LEFT_BRACE,
    ZEN_TOKEN_RIGHT_BRACE,

    ZEN_TOKEN_LEFT_SQUARE_BRACKET,
    ZEN_TOKEN_RIGHT_SQUARE_BRACKET,

    ZEN_TOKEN_CARET_EQUAL,
    ZEN_TOKEN_CARET,

    ZEN_TOKEN_VERTICAL_BAR_2,
    ZEN_TOKEN_VERTICAL_BAR_EQUAL,
    ZEN_TOKEN_VERTICAL_BAR,

    ZEN_TOKEN_TILDE_EQUAL,
    ZEN_TOKEN_TILDE,

    ZEN_TOKEN_IDENTIFIER,

    /* Keywords */

    ZEN_TOKEN_KEYWORD_DO, // REMOVE
    ZEN_TOKEN_KEYWORD_IF,
    ZEN_TOKEN_KEYWORD_IN, // REMOVE
    ZEN_TOKEN_KEYWORD_IS,
    ZEN_TOKEN_KEYWORD_OR,

    ZEN_TOKEN_KEYWORD_AND,
    ZEN_TOKEN_KEYWORD_FOR,
    // ZEN_TOKEN_KEYWORD_NAN,
    ZEN_TOKEN_KEYWORD_NEW,
    ZEN_TOKEN_KEYWORD_TRY,
    ZEN_TOKEN_KEYWORD_VAR,

    ZEN_TOKEN_KEYWORD_CASE, // REMOVE
    ZEN_TOKEN_KEYWORD_ELSE,
    ZEN_TOKEN_KEYWORD_ENUM,
    ZEN_TOKEN_KEYWORD_NULL,
    ZEN_TOKEN_KEYWORD_THEN, // REMOVE
    ZEN_TOKEN_KEYWORD_THIS,
    ZEN_TOKEN_KEYWORD_TRUE,
    ZEN_TOKEN_KEYWORD_WITH,

    ZEN_TOKEN_KEYWORD_BREAK,
    ZEN_TOKEN_KEYWORD_CATCH,
    ZEN_TOKEN_KEYWORD_CLASS,
    ZEN_TOKEN_KEYWORD_FALSE,
    ZEN_TOKEN_KEYWORD_FINAL,
    ZEN_TOKEN_KEYWORD_THROW,
    ZEN_TOKEN_KEYWORD_WHILE,

    ZEN_TOKEN_KEYWORD_ASSERT,
    ZEN_TOKEN_KEYWORD_IMPORT,
    ZEN_TOKEN_KEYWORD_NATIVE,
    ZEN_TOKEN_KEYWORD_PUBLIC,
    ZEN_TOKEN_KEYWORD_RETURN,
    ZEN_TOKEN_KEYWORD_SECRET,
    ZEN_TOKEN_KEYWORD_STATIC,
    // ZEN_TOKEN_KEYWORD_SWITCH,

    ZEN_TOKEN_KEYWORD_ABSTRACT,
    // ZEN_TOKEN_KEYWORD_DEFAULT,
    ZEN_TOKEN_KEYWORD_FINALLY,
    // ZEN_TOKEN_KEYWORD_PACKAGE,
    ZEN_TOKEN_KEYWORD_PRIVATE,
    ZEN_TOKEN_KEYWORD_CONTINUE,
    ZEN_TOKEN_KEYWORD_FUNCTION,

    ZEN_TOKEN_KEYWORD_SYNCHRONIZE,

    ZEN_TOKEN_INTEGER_LITERAL,
    ZEN_TOKEN_STRING_LITERAL,
    ZEN_TOKEN_FLOATING_POINT_LITERAL
};

/**
 * @memberof TokenType
 */
typedef enum zen_TokenType_t zen_TokenType_t;

#endif /* COM_ONECUBE_ZEN_COMPILER_LEXER_TOKEN_TYPE_H */