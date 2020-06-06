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

// Sunday, November 05, 2017

#ifndef KUSH_TOKEN_STREAM_H
#define KUSH_TOKEN_STREAM_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/compiler.h>
#include <kush/lexer.h>
#include <kush/Token.h>
#include <kush/TokenType.h>

/*******************************************************************************
 * TokenStream                                                           *
 *******************************************************************************/

/* NOTE: The lexical error object received by this call back is immediately
 * destroyed as soon as it returns. Therefore, do not maintain a reference to
 * it.
 */
// typedef void (*k_OnLexicalError_t)(k_LexicalError_t* error);

struct k_TokenStreat {

    k_Compiler_t* compiler;

    /**
     * The lexer which recognizes and produces tokens on this
     * stream.
     */
    k_Lexer_t* lexer;

    /**
     * The list of all the tokens recognized by the lexer so
     * far. It is considered a complete view of the input source
     * once the lexer recognizes the end-of-stream token.
     */
    jtk_ArrayList_t* tokens;

    /**
     * The index of the current token.
     */
    int32_t p;

    /**
     * Determines whether the end-of-stream token has been
     * produced by the lexer.
     */
    bool hitEndOfStream;

    /**
     * The channel on which the token stream filters tokens
     * from.
     */
    k_TokenChannel_t channel;

    jtk_ArrayList_t* trash;
};

typedef struct k_TokenStreat k_TokenStreat;

k_TokenStreat* k_TokenStreanew(k_Compiler_t* compiler, k_Lexer_t* lexer,
    k_TokenChannel_t channel);
void k_TokenStreadelete(k_TokenStreat* stream);
void k_TokenStreareset(k_TokenStreat* stream);
int32_t k_TokenStreagetIndex(k_TokenStreat* stream);
int32_t k_TokenStreagetSize(k_TokenStreat* stream);
void k_TokenStreaconsume(k_TokenStreat* stream);
bool k_TokenStreasynchronize(k_TokenStreat* stream, int32_t i);
int32_t k_TokenStreafetch(k_TokenStreat* stream, int32_t n);
k_Token_t* k_TokenStreagetToken(k_TokenStreat* stream, int32_t index);
jtk_ArrayList_t* k_TokenStreagetTokens(k_TokenStreat* stream, int32_t startIndex, int32_t stopIndex);
k_TokenType_t k_TokenStreala(k_TokenStreat* stream, int32_t i);
k_Token_t* k_TokenStrealt(k_TokenStreat* stream, int32_t k);
void k_TokenStreainitialize(k_TokenStreat* stream);
int32_t k_TokenStreagetNextTokenOnChannel(k_TokenStreat* stream, int32_t i, k_TokenChannel_t channel);
int32_t k_TokenStreagetPreviousTokenOnChannel(k_TokenStreat* stream, int32_t i, k_TokenChannel_t channel);
void k_TokenStreafill(k_TokenStreat* stream);
uint8_t* k_TokenStreagetSourceName(k_TokenStreat* stream, int32_t* size);
k_Lexer_t* k_TokenStreagetLexer(k_TokenStreat* stream);
uint8_t* k_TokenStreagetText(k_TokenStreat* stream, int32_t startIndex, int32_t stopIndex, int32_t* size);
int32_t k_TokenStreagetNumberOfTokens(k_TokenStreat* stream, k_TokenChannel_t channel);

#endif /* KUSH_TOKEN_STREAM_H */
