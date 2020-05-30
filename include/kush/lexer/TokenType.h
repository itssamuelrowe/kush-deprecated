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

// Saturday, November 04, 2017

#ifndef KUSH_COMPILER_LEXER_TOKEN_TYPE_H
#define KUSH_COMPILER_LEXER_TOKEN_TYPE_H

#include <kush/Configuration.h>

/*******************************************************************************
 * TokenType                                                                   *
 *******************************************************************************/

/**
 * @class TokenType
 * @ingroup k_compiler_lexer
 * @author Samuel Rowe
 * @since KUSH 1.0
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
    KUSHKUSH_TOKEN_TILDE,

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
    
    KUSH_TOKEN_KEYWORD_LET,
    
    KUSH_TOKEN_KEYWORD_NEW,
    KUSH_TOKEN_KEYWORD_NULL,
    
    KUSH_TOKEN_KEYWORD_RETURN,
    
    KUSH_TOKEN_KEYWORD_STRUCT
    
    KUSH_TOKEN_KEYWORD_THIS,
    KUSH_TOKEN_KEYWORD_THROW,
    KUSH_TOKEN_KEYWORD_TRUE,
    KUSH_TOKEN_KEYWORD_TRY,
    
    KUSH_TOKEN_KEYWORD_VAR,
    KUSH_TOKEN_KEYWORD_VOID,
    
    KUSH_TOKEN_KEYWORD_WHILE,
		
    KUSH_TOKEN_INTEGER_LITERAL,
    KUSH_TOKEN_STRING_LITERAL,
    KUSH_TOKEN_FLOATING_POINT_LITERAL
};

/**
 * @memberof TokenType
 */
typedef enum k_TokenType_t k_TokenType_t;

#endif /* KUSH_COMPILER_LEXER_TOKEN_TYPE_H */
