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

// Sunday, December 03, 2017

#ifndef KUSH_COMPILER_PARSER_PARSER_ERROR_H
#define KUSH_COMPILER_PARSER_PARSER_ERROR_H

#include <kush/Configuration.h>

/*******************************************************************************
 * ParserError                                                                 *
 *******************************************************************************/

/**
 * @class ParserError
 * @ingroup k_compiler_parser
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_ParserError_t {
    char* message;
};

/**
 * @memberof ParserError
 */
typedef struct k_ParserError_t k_ParserError_t;

// Constructor

/**
 * @memberof ParserError
 */
k_ParserError_t* k_ParserError_new();

// Destructor

/**
 * @memberof ParserError
 */
void k_ParserError_delete(k_ParserError_t* error);

#endif /* KUSH_COMPILER_PARSER_PARSER_ERROR_H */
