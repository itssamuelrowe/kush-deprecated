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
    k_TokenChannel_t m_channel;
    k_TokenType_t m_type;
    uint8_t* m_text;
    int32_t m_length;
    int32_t m_startIndex;
    int32_t m_stopIndex;
    int32_t m_startLine;
    int32_t m_stopLine;
    int32_t m_startColumn;
    int32_t m_stopColumn;
    int32_t m_index;
    const char* m_file;
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
