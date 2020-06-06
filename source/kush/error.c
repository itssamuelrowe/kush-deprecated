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

#include <kush/support/Error.h>

/*******************************************************************************
 * Error                                                                       *
 *******************************************************************************/

// Constructor

k_Error_t* k_Error_new(k_ErrorCode_t code, k_Token_t* token) {
    return k_Error_newEx(code, token, k_TOKEN_UNKNOWN);
}

k_Error_t* k_Error_newEx(k_ErrorCode_t code, k_Token_t* token,
    k_TokenType_t expected) {
    k_Error_t* error = jtallocate(k_Error_t, 1);
    error->code = code;
    error->token = token;
    error->expected = expected;

    return error;
}

// Destructor

void k_Error_delete(k_Error_t* error) {
    jtk_Assert_assertObject(error, "The specified error is null.");

    jtdeallocate(error);
}
