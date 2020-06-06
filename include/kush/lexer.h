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

#ifndef KUSH_COMPILER_LEXER_LEXER_H
#define KUSH_COMPILER_LEXER_LEXER_H

#include <jtk/collection/list/ArrayList.h>
#include <jtk/collection/queue/ArrayQueue.h>
#include <jtk/collection/stack/ArrayStack.h>
#include <jtk/core/StringBuilder.h>
#include <jtk/io/InputStream.h>

#include <kush/Configuration.h>
#include <kush/Compiler.h>
#include <kush/lexer/LexerError.h>
#include <kush/lexer/Token.h>
#include <kush/lexer/TokenType.h>
#include <kush/lexer/TokenChannel.h>

#define KUSH_LEXER_DEFAULT_CHANNEL 0
#define KUSH_LEXER_HIDDEN_CHANNEL 1

#define KUSH_END_OF_STREAM -1

/* Forward Reference */

typedef struct k_ErrorHandler_t k_ErrorHandler_t;

/*******************************************************************************
 * Lexer                                                                       *
 *******************************************************************************/

/**
 * Generates a stream of tokens from an input character stream.
 * The tokens are recognized based on fixed lexical patterns
 * as described by KUSH.
 *
 * @author Samuel Rowe
 * @since  KUSH 1.0
 */
struct k_Lexer_t {

    k_Compiler_t* compiler;

    /**
     * The input stream of characters.
     */
    jtk_InputStream_t* inputStream;

    /**
     * The character at LA(1), this field is always updated
     * by k_Lexer_consume(...).
     */
    int32_t la1;

    /**
     * The input cursor under which the current look ahead
     * character is located.
     */
    int32_t index;

    /**
     * The zero based line number at which the lexer is currently
     * reading.
     */
    int32_t line;

    /**
     * The zero based column index within the line, where the
     * lexer is currently reading.
     */
    int32_t column;

    /**
     * The starting index of the current token in the input
     * stream, inclusive.
     */
    int32_t startIndex;

    /**
     * The zero based line number at which the current
     * token begins, inclusive.
     */
    int32_t startLine;

    /**
     * The zero based column at which the current token
     * begins. It is always relative to the starting line.
     */
    int32_t startColumn;

    /**
     * Determines whether the lexer has reached the end of
     * the input stream.
     */
    int32_t hitEndOfStream:1;

    /**
     * The token that was most recently emitted.
     */
    k_Token_t* token;

    /**
     * The channel on which the next recognized
     * token will be created on.
     */
    k_TokenChannel_t  channel;

    /**
     * The text consumed so far to recognize the next
     * token.
     */
    jtk_StringBuilder_t* text;

    /**
     * The token type of the next recognized token.
     */
    k_TokenType_t type;

    /**
     * A buffer to store emitted tokens.
     *
     * A single call to k_Lexer_nextToken() may result in
     * emission of multiple tokens. Therefore, the lexer
     * buffers up tokens.
     */
    jtk_ArrayQueue_t* tokens;

    k_ErrorCode_t errorCode;
};

/**
 * @memberof Lexer
 */
typedef struct k_Lexer_t k_Lexer_t;

const uint8_t* k_Lexer_getLiteralName(k_TokenType_t type);

// Constructor

/**
 * Creates and returns a new lexer.
 *
 * @return A new lexer.
 */
k_Lexer_t* k_Lexer_new(k_Compiler_t* compiler);

// Destructor

/**
 * Destroys the specified lexer.
 *
 * @param lexer
 *        The lexer to destroy.
 */
void k_Lexer_delete(k_Lexer_t* lexer);

/**
 * The primary interface for the Lexer class. It uses the lookahead
 * character (character under the input cursor) to route control
 * flow to the appropriate recognition function. And returns the
 * next token recognized from the input stream.
 *
 * @param  lexer
 *         The lexer.
 * @param  lexerError
 *         An error that represents a lexical mismatch.
 *         It is initialized by this function, if an error
 *         is encountered; ignored, otherwise.
 * @return The next recognized token.
 */
k_Token_t* k_Lexer_nextToken(k_Lexer_t* lexer);

#endif /* KUSH_COMPILER_LEXER_LEXER_H */
