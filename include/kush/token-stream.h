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

#ifndef TOKEN_STREAM_H
#define TOKEN_STREAM_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/configuration.h>
#include <kush/compiler.h>
#include <kush/lexer.h>


/*******************************************************************************
 * TokenStream                                                           *
 *******************************************************************************/

/* NOTE: The lexical error object received by this call back is immediately
 * destroyed as soon as it returns. Therefore, do not maintain a reference to
 * it.
 */
// typedef void (*k_OnLexicalError_t)(k_LexicalError_t* error);

struct k_TokenStream_t {

    Compiler* compiler;

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
    TokenChannel channel;

    jtk_ArrayList_t* trash;
};

typedef struct k_TokenStream_t k_TokenStream_t;

k_TokenStream_t* tokenStreamNew(Compiler* compiler, k_Lexer_t* lexer,
    TokenChannel channel);
void k_TokenStream_delete(k_TokenStream_t* stream);
void k_TokenStream_reset(k_TokenStream_t* stream);
int32_t k_TokenStream_getIndex(k_TokenStream_t* stream);
int32_t k_TokenStream_getSize(k_TokenStream_t* stream);
void k_TokenStream_consume(k_TokenStream_t* stream);
bool k_TokenStream_synchronize(k_TokenStream_t* stream, int32_t i);
int32_t k_TokenStream_fetch(k_TokenStream_t* stream, int32_t n);
Token* k_TokenStream_getToken(k_TokenStream_t* stream, int32_t index);
jtk_ArrayList_t* k_TokenStream_getTokens(k_TokenStream_t* stream, int32_t startIndex, int32_t stopIndex);
TokenType k_TokenStream_la(k_TokenStream_t* stream, int32_t i);
Token* k_TokenStream_lt(k_TokenStream_t* stream, int32_t k);
void k_TokenStream_initialize(k_TokenStream_t* stream);
int32_t k_TokenStream_getNextTokenOnChannel(k_TokenStream_t* stream, int32_t i, TokenChannel channel);
int32_t k_TokenStream_getPreviousTokenOnChannel(k_TokenStream_t* stream, int32_t i, TokenChannel channel);
void k_TokenStream_fill(k_TokenStream_t* stream);
uint8_t* k_TokenStream_getSourceName(k_TokenStream_t* stream, int32_t* size);
k_Lexer_t* k_TokenStream_getLexer(k_TokenStream_t* stream);
uint8_t* k_TokenStream_getText(k_TokenStream_t* stream, int32_t startIndex, int32_t stopIndex, int32_t* size);
int32_t k_TokenStream_getNumberOfTokens(k_TokenStream_t* stream, TokenChannel channel);

#endif /* TOKEN_STREAM_H */
