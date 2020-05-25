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
#include <kush/lexer/TokenStream.h>

k_TokenStream_t* k_TokenStream_new(k_Compiler_t* compiler,
    k_Lexer_t* lexer, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(lexer, "The specified lexer is null.");

    k_TokenStream_t* stream = k_Memory_allocate(k_TokenStream_t, 1);
    stream->m_compiler = compiler;
    stream->m_lexer = lexer;
    stream->m_tokens = jtk_ArrayList_newWithCapacity(128);
    stream->m_p = -1;
    stream->m_hitEndOfStream = false;
    stream->m_channel = channel;
    stream->m_trash = jtk_ArrayList_new();

    return stream;
}

void k_TokenStream_destroyStaleTokens(k_TokenStream_t* stream) {
}

// TODO: The tokens must be destroyed!
void k_TokenStream_delete(k_TokenStream_t* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");

    jtk_ArrayList_delete(stream->m_tokens);
    jtk_Memory_deallocate(stream);
}

void k_TokenStream_reset(k_TokenStream_t* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");

    jtk_ArrayList_clear(stream->m_tokens);
    stream->m_p = -1;
    stream->m_hitEndOfStream = false;
}

int32_t k_TokenStream_getIndex(k_TokenStream_t* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");
    return stream->m_p;
}

int32_t k_TokenStream_getSize(k_TokenStream_t* stream) {
    jtk_Assert_assertObject(stream, "The specified token stream is null.");
    return jtk_ArrayList_getSize(stream->m_tokens);
}

void k_TokenStream_consume(k_TokenStream_t* stream) {
    bool skip;
    if (stream->m_p >= 0) {
        if (stream->m_hitEndOfStream) {
            /* The end-of-stream token can be consumed only once.
             * Should we check this condition?
             */
            skip = stream->m_p < (jtk_ArrayList_getSize(stream->m_tokens) - 1);
        }
        else {
            /* Have we consumed all the buffered tokens? */
            skip = stream->m_p < jtk_ArrayList_getSize(stream->m_tokens);
        }
    }
    else {
        /* Currently, the token stream has no tokens to consume. */
        skip = false;
    }

    jtk_Assert_assertFalse((!skip && (k_TokenStream_la(stream, 1) == ZEN_TOKEN_END_OF_STREAM)), "...");

    bool hasToken = k_TokenStream_synchronize(stream, stream->m_p + 1);
    if (hasToken) {
        stream->m_p = k_TokenStream_getNextTokenOnChannel(stream, stream->m_p + 1, stream->m_channel);
    }
}

bool k_TokenStream_synchronize(k_TokenStream_t* stream, int32_t i) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");
    jtk_Assert_assertTrue(i >= 0, "The specified index is invalid.");

    int32_t n = i - jtk_ArrayList_getSize(stream->m_tokens) + 1;
    bool result = true;
    if (n > 0) {
        int32_t fetched = k_TokenStream_fetch(stream, n);
        result = fetched >= n;
    }
    return result;
}

int32_t k_TokenStream_fetch(k_TokenStream_t* stream, int32_t n) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    if (stream->m_hitEndOfStream) {
        return 0;
    }

    int32_t oldSize = jtk_ArrayList_getSize(stream->m_tokens);
    int32_t i;
    for (i = 0; i < n; i++) {
        k_Token_t* token = k_Lexer_nextToken(stream->m_lexer);
        k_Token_setIndex(token, oldSize + i);
        jtk_ArrayList_add(stream->m_tokens, token);
        jtk_ArrayList_add(stream->m_trash, token);

        if (k_Token_getType(token) == ZEN_TOKEN_END_OF_STREAM) {
            stream->m_hitEndOfStream = true;
            return i + 1;
        }
    }
    return n;
}

k_Token_t* k_TokenStream_getToken(k_TokenStream_t* stream, int32_t index) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Index-out-of-range errors are checked by the
     * jtk_ArrayList_t class.
     */
    return jtk_ArrayList_getValue(stream->m_tokens, index);
}

jtk_ArrayList_t* k_TokenStream_getTokens(k_TokenStream_t* stream,
    int32_t startIndex, int32_t stopIndex) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    int32_t size = jtk_ArrayList_getSize(stream->m_tokens);
    jtk_Arrays_checkRange(size, startIndex, stopIndex);

    k_TokenStream_initialize(stream);
    jtk_ArrayList_t* result = jtk_ArrayList_new();
    int32_t i;
    for (i = startIndex; i < stopIndex; i++) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
        jtk_ArrayList_add(result, token);
    }
    /* The user is responsible for the destruction of
     * this list.
     */
    return result;
}

k_TokenType_t k_TokenStream_la(k_TokenStream_t* stream, int32_t i) {
    return k_TokenStream_lt(stream, i)->m_type;
}

k_Token_t* k_TokenStream_lt(k_TokenStream_t* stream, int32_t k) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    k_TokenStream_initialize(stream);
    k_Token_t* token = NULL;
    if (k != 0) {
        if (k < 0) {
            if ((stream->m_p - k) >= 0) {
                int32_t i = stream->m_p;
                int32_t n = 1;
                while ((n <= k) && (i > 0)) {
                    i = k_TokenStream_getPreviousTokenOnChannel(stream, i - 1, stream->m_channel);
                    n++;
                }
                if (i >= 0) {
                    token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
                }
            }
        }
        else {
            int32_t i = stream->m_p;
            int32_t n = 1;
            while (n < k) {
                bool hasToken = k_TokenStream_synchronize(stream, i + 1);
                if (hasToken) {
                    i = k_TokenStream_getNextTokenOnChannel(stream, i + 1, stream->m_channel);
                }
                n++;
            }
            token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
        }
    }
    return token;
}

void k_TokenStream_initialize(k_TokenStream_t* stream) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    if (stream->m_p == -1) {
        k_TokenStream_synchronize(stream, 0);
        stream->m_p = k_TokenStream_getNextTokenOnChannel(stream, 0, stream->m_channel);
    }
}

int32_t k_TokenStream_getPreviousTokenOnChannel(k_TokenStream_t* stream,
    int32_t i, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Ensure that the token stream has buffered, at least,
     * tokens till the requested index.
     */
    k_TokenStream_synchronize(stream, i);
    int32_t size = jtk_ArrayList_getSize(stream->m_tokens);
    if (i >= size) {
        /* In case the synchronization failed to retrieve the
         * requested number of tokens, return the index of the
         * end-of-stream token.
         */
        return size - 1;
    }

    while (i >= 0) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
        if ((k_Token_getType(token) == ZEN_TOKEN_END_OF_STREAM) ||
            (k_Token_getChannel(token) == channel)) {
            return i;
        }
        i--;
    }

    return -1;
}

int32_t k_TokenStream_getNextTokenOnChannel(k_TokenStream_t* stream,
    int32_t i, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Ensure that the token stream has buffered, at least,
     * tokens till the requested index.
     */
    k_TokenStream_synchronize(stream, i);
    int32_t size = jtk_ArrayList_getSize(stream->m_tokens);
    if (i >= size) {
        /* In case the synchronization failed to retrieve the
         * requested number of tokens, return the index of the
         * end-of-stream token.
         */
        return size - 1;
    }

    k_Token_t* token = jtk_ArrayList_getValue(stream->m_tokens, i);
    while (k_Token_getChannel(token) != channel) {
        /* In case the token stream has reached the end-of-stream,
         * return the index of the end-of-stream token.
         */
        if (k_Token_getType(token) == ZEN_TOKEN_END_OF_STREAM) {
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
        k_TokenStream_synchronize(stream, i);
        /* Update the token variable for next iteration. */
        token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
    }
    return i;
}

void k_TokenStream_fill(k_TokenStream_t* stream) {
    k_TokenStream_initialize(stream);
    /* The token stream tries to buffer a 1000 tokens
     * at each iteration. This is repeated until the token
     * stream fails to fetch the quota, which indicates
     * complete tokenization of the input stream.
     */
    int32_t blockSize = 1000;
    int32_t fetched;
    do {
        fetched = k_TokenStream_fetch(stream, blockSize);
    } while (fetched == blockSize);
}

int32_t k_TokenStream_getNumberOfTokens(k_TokenStream_t* stream,
    k_TokenChannel_t channel) {
    jtk_Assert_assertObject(stream, "The specified token source is null.");

    /* Fetch all the tokens from the input stream before computing
     * the number of tokens on the specified channel.
     */
    k_TokenStream_fill(stream);

    int32_t n = 0;
    int32_t size = jtk_ArrayList_getSize(stream->m_tokens);
    int32_t i;
    for (i = 0; i < size; i++) {
        k_Token_t* token = (k_Token_t*)jtk_ArrayList_getValue(stream->m_tokens, i);
        if (k_Token_getChannel(token) == channel) {
            n++;
        }
    }
    return n;
}