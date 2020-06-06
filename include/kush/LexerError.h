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

// Friday, November 04, 2017

#ifndef KUSH_COMPILER_LEXER_LEXER_ERROR_H
#define KUSH_COMPILER_LEXER_LEXER_ERROR_H

#include <kush/Configuration.h>

/*******************************************************************************
 * LexerError                                                                  *
 *******************************************************************************/

/**
 * @class LexerError
 * @ingroup k_compiler_lexer
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_LexerError_t {
    char* message;
    char* path;
    int32_t line;
    int32_t column;
};

/**
 * @memberof LexerError
 */
typedef struct k_LexerError_t k_LexerError_t;

// Constructor

/**
 * @memberof LexerError
 */
k_LexerError_t* k_LexerError_new(const char* message, const char* path,
    int32_t line, int32_t column);

// Destructor

/**
 * @memberof LexerError
 */
void k_LexerError_delete(k_LexerError_t* error);

#endif /* KUSH_COMPILER_LEXER_LEXER_ERROR_H */
