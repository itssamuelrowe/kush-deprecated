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

#ifndef KUSH_COMPILER_PARSER_PARSER_H
#define KUSH_COMPILER_PARSER_PARSER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/lexer/TokenStream.h>

/*******************************************************************************
 * Parser                                                                      *
 *******************************************************************************/

/**
 * @class Parser
 * @ingroup k_compiler_parser
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Parser_t {
    k_Compiler_t* compiler;
    k_TokenStreat* tokens;
    k_TokenType_t* followSet;
    int32_t followSetSize;
    int32_t followSetCapacity;
    bool recovery;
    k_ASTNodeType_t mainComponent;
};

/**
 * @memberof Parser
 */
typedef struct k_Parser_t k_Parser_t;

// Constructor

/**
 * @memberof Parser
 */
k_Parser_t* k_Parser_new(k_Compiler_t* compiler, k_TokenStreat* tokens);

// Destructor

/**
 * @memberof Parser
 */
void k_Parser_delete(k_Parser_t* parser);

// Rule

/**
 * @memberof Parser
 */
const char* k_Parser_getRuleName(k_ASTNodeType_t type);

// Parse

/**
 * @memberof Parser
 */
void k_Parser_parse(k_Parser_t* parser);

// Reset

/**
 * @memberof Parser
 */
void k_Parser_reset(k_Parser_t* parser, k_TokenStreat* tokens);

#endif /* KUSH_COMPILER_PARSER_PARSER_H */
