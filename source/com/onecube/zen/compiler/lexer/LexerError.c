/*
 * Copyright 2018-2019 OneCube
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

#include <string.h>
#include <com/onecube/zen/LexerError.h>

zen_LexerError_t* zen_LexerError_new(const char* message, const char* path, int32_t line, int32_t column) {
    zen_LexerError_t* error = zen_Memory_allocate(zen_LexerError_t, 1);
    error->m_message = (message == NULL)? strdup("No error message") : strdup(message);
    error->m_path = strdup(path);
    error->m_line = line;
    error->m_column = column;
    
    return error;
}

void zen_LexerError_delete(zen_LexerError_t* error) {
    jtk_Assert_assertObject(error, "The specified lexer error is null.");
    free(error->m_message);
    zen_Memory_deallocate(error);
}