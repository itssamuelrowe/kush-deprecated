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

#ifndef COM_ONECUBE_ZEN_COMPILER_SUPPORT_ERROR_H
#define COM_ONECUBE_ZEN_COMPILER_SUPPORT_ERROR_H

#include <kush/lexer/Token.h>
#include <kush/support/ErrorCode.h>

/*******************************************************************************
 * Error                                                                       *
 *******************************************************************************/

/**
 * @class Error
 * @ingroup k_compiler_support
 * @author Samuel Rowe
 * @since Zen 1.0
 */
struct k_Error_t {
    k_ErrorCode_t m_code;
    k_Token_t* m_token;
    k_TokenType_t m_expected;
};

/**
 * @memberof Error
 */
typedef struct k_Error_t k_Error_t;

// Constructor

k_Error_t* k_Error_new(k_ErrorCode_t errorCode, k_Token_t* token);
k_Error_t* k_Error_newEx(k_ErrorCode_t errorCode, k_Token_t* token, k_TokenType_t expected);

// Destructor

void k_Error_delete(k_Error_t* error);

#endif /* COM_ONECUBE_ZEN_COMPILER_SUPPORT_ERROR_H */