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

#include <jtk/collection/array/Arrays.h>
#include <kush/TokenStream.h>

k_TokenStreat* k_TokenStreanew(k_Compiler_t* compiler,
                                   k_Lexer_t* lexer, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(lexer, "The specified lexer is null.");

    k_TokenStreat* stream = allocate(k_TokenStreat, 1);
    stream->compiler = compiler;
    stream->lexer = lexer;
    stream->tokens = jtk_ArrayList_newWithCapacity(128);
    stream->p = -1;
    stream->hitEndOfStream = false;
    stream->channel = channel;
    stream->trash = jtk_ArrayList_new();

    return stream;
}

void k_TokenStreadestroyStaleTokens(k_TokenStreat* stream) {
}

// TODO: The tokens must be destroyed!
void k_TokenStreadelete(k_TokenStreat* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");

    jtk_ArrayList_delete(stream->tokens);
    jtdeallocate(stream);
}

void k_TokenStreareset(k_TokenStreat* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");

    jtk_ArrayList_clear(stream->tokens);
    stream->p = -1;
    stream->hitEndOfStream = false;
}

int32_t k_TokenStreagetIndex(k_TokenStreat* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");
    return stream->p;
}

int32_t k_TokenStreagetSize(k_TokenStreat* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");
    return jtk_ArrayList_getSize(stream->tokens);
}

void k_TokenStreaconsume(k_TokenStreat* stream) {
    bool skip;
    if (stream->p >= 0) {
        if (stream->hitEndOfStream) {
            /* The end-of-stream token can be consumed only once.
             * Should we check this condition?
             */
            skip = stream->p < (jtk_ArrayList_getSize(stream->tokens) - 1);
        }
        else {
            /* Have we consumed all the buffered tokens? */
            skip = stream->p < jtk_ArrayList_getSize(stream->tokens);
        }
    }
    else {
        /* Currently, the token stream has no tokens to consume. */
        skip = false;
    }

    jtk_Assert_assertFalse((!skip && (k_TokenStreala(stream, 1) == KUSH_TOKEN_END_OF_STREAM)), "...");

    bool hasToken = k_TokenStreasynchronize(stream, stream->p + 1);
    if (hasToken) {
        stream->p = k_TokenStreagetNextTokenOnChannel(stream, stream->p + 1, stream->channel);
    }
}

bool k_TokenStreasynchronize(k_TokenStreat* stream, int32_t i) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");
    jtk_Assert_assertTrue(i >= 0, "The specified index is invalid.");

    int32_t n = i - jtk_ArrayList_getSize(stream->tokens) + 1;
    bool result = true;
    if (n > 0) {
        int32_t fetched = k_TokenStreafetch(stream, n);
        result = fetched >= n;
    }
    return result;
}

int32_t k_TokenStreafetch(k_TokenStreat* stream, int32_t n) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    if (stream->hitEndOfStream) {
        return 0;
    }

    int32_t oldSize = jtk_ArrayList_getSize(stream->tokens);
    int32_t i;
    for (i = 0; i < n; i++) {
        k_Token_t* token = k_Lexer_nextToken(stream->lexer);
        k_Token_setIndex(token, oldSize + i);
        jtk_ArrayList_add(stream->tokens, token);
        jtk_ArrayList_add(stream->trash, token);

        if (k_Token_getType(token) == KUSH_TOKEN_END_OF_STREAM) {
            stream->hitEndOfStream = true;
            return i + 1;
        }
    }
    return n;
}

k_Token_t* k_TokenStreagetToken(k_TokenStreat* stream, int32_t index) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Index-out-of-range errors are checked by the
     * jtk_ArrayList_t class.
     */
    return jtk_ArrayList_getValue(stream->tokens, index);
}

jtk_ArrayList_t* k_TokenStreagetTokens(k_TokenStreat* stream,
        int32_t startIndex, int32_t stopIndex) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    int32_t size = jtk_ArrayList_getSize(stream->tokens);
    jtk_Arrays_checkRange(size, startIndex, stopIndex);

    k_TokenStreainitialize(stream);
    jtk_ArrayList_t* result = jtk_ArrayList_new();
    int32_t i;
    for (i = startIndex; i < stopIndex; i++) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
        jtk_ArrayList_add(result, token);
    }
    /* The user is responsible for the destruction of
     * this list.
     */
    return result;
}

k_TokenType_t k_TokenStreala(k_TokenStreat* stream, int32_t i) {
    return k_TokenStrealt(stream, i)->type;
}

k_Token_t* k_TokenStrealt(k_TokenStreat* stream, int32_t k) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    k_TokenStreainitialize(stream);
    k_Token_t* token = NULL;
    if (k != 0) {
        if (k < 0) {
            if ((stream->p - k) >= 0) {
                int32_t i = stream->p;
                int32_t n = 1;
                while ((n <= k) && (i > 0)) {
                    i = k_TokenStreagetPreviousTokenOnChannel(stream, i - 1, stream->channel);
                    n++;
                }
                if (i >= 0) {
                    token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
                }
            }
        }
        else {
            int32_t i = stream->p;
            int32_t n = 1;
            while (n < k) {
                bool hasToken = k_TokenStreasynchronize(stream, i + 1);
                if (hasToken) {
                    i = k_TokenStreagetNextTokenOnChannel(stream, i + 1, stream->channel);
                }
                n++;
            }
            token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
        }
    }
    return token;
}

void k_TokenStreainitialize(k_TokenStreat* stream) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    if (stream->p == -1) {
        k_TokenStreasynchronize(stream, 0);
        stream->p = k_TokenStreagetNextTokenOnChannel(stream, 0, stream->channel);
    }
}

int32_t k_TokenStreagetPreviousTokenOnChannel(k_TokenStreat* stream,
        int32_t i, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Ensure that the token stream has buffered, at least,
     * tokens till the requested index.
     */
    k_TokenStreasynchronize(stream, i);
    int32_t size = jtk_ArrayList_getSize(stream->tokens);
    if (i >= size) {
        /* In case the synchronization failed to retrieve the
         * requested number of tokens, return the index of the
         * end-of-stream token.
         */
        return size - 1;
    }

    while (i >= 0) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
        if ((k_Token_getType(token) == KUSH_TOKEN_END_OF_STREAM) ||
                (k_Token_getChannel(token) == channel)) {
            return i;
        }
        i--;
    }

    return -1;
}

int32_t k_TokenStreagetNextTokenOnChannel(k_TokenStreat* stream,
        int32_t i, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Ensure that the token stream has buffered, at least,
     * tokens till the requested index.
     */
    k_TokenStreasynchronize(stream, i);
    int32_t size = jtk_ArrayList_getSize(stream->tokens);
    if (i >= size) {
        /* In case the synchronization failed to retrieve the
         * requested number of tokens, return the index of the
         * end-of-stream token.
         */
        return size - 1;
    }

    k_Token_t* token = jtk_ArrayList_getValue(stream->tokens, i);
    while (k_Token_getChannel(token) != channel) {
        /* In case the token stream has reached the end-of-stream,
         * return the index of the end-of-stream token.
         */
        if (k_Token_getType(token) == KUSH_TOKEN_END_OF_STREAM) {
            return i;
        }

        /* The token source failed to find a token on the
         * requested channel at this index. Increment it
         * to the next index.
         */
        i++;
        /* Ensure that the token stream has buffered, at least,
         * tokens till the new index.
         */
        k_TokenStreasynchronize(stream, i);
        /* Update the token variable for next iteration. */
        token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
    }
    return i;
}

void k_TokenStreafill(k_TokenStreat* stream) {
    k_TokenStreainitialize(stream);
    /* The token stream tries to buffer a 1000 tokens
     * at each iteration. This is repeated until the token
     * stream fails to fetch the quota, which indicates
     * complete tokenization of the input stream.
     */
    int32_t blockSize = 1000;
    int32_t fetched;
    do {
        fetched = k_TokenStreafetch(stream, blockSize);
    } while (fetched == blockSize);
}

int32_t k_TokenStreagetNumberOfTokens(k_TokenStreat* stream,
                                        k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Fetch all the tokens from the input stream before computing
     * the number of tokens on the specified channel.
     */
    k_TokenStreafill(stream);

    int32_t n = 0;
    int32_t size = jtk_ArrayList_getSize(stream->tokens);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->tokens, i);
        if (k_Token_getChannel(token) == channel) {
            n++;
        }
    }
    return n;
}
