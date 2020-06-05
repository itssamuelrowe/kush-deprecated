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

#include <jtk/core/CString.h>
#include <kush/lexer/Token.h>

k_Token_t* k_Token_new(
    k_TokenChannel_t channel,
    k_TokenType_t type,
    const uint8_t* text,
    int32_t length,
    int32_t startIndex,
    int32_t stopIndex,
    int32_t startLine,
    int32_t stopLine,
    int32_t startColumn,
    int32_t stopColumn,
    const char* file) {
    k_Token_t* token = k_Memory_allocate(k_Token_t, 1);
    token->m_channel = channel;
    token->m_type = type;
    token->m_text = jtk_CString_newEx(text, length);
    token->m_length = length; // This is the length of the text representation!
    token->m_startIndex = startIndex;
    token->m_stopIndex = stopIndex;
    token->m_startLine = startLine;
    token->m_stopLine = stopLine;
    token->m_startColumn = startColumn;
    token->m_stopColumn = stopColumn;
    token->m_file = file;

    return token;
}

void k_Token_delete(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    jtk_CString_delete(token->m_text);
    jtk_Memory_deallocate(token);
}

k_TokenChannel_t k_Token_getChannel(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_channel;
}

void k_Token_setChannel(k_Token_t* token, k_TokenChannel_t channel) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_channel = channel;
}

k_TokenType_t k_Token_getType(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_type;
}

void k_Token_setType(k_Token_t* token, k_TokenType_t type) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_type = type;
}


void k_Token_setText(k_Token_t* token, const uint8_t* text, int32_t length) {
    jtk_Assert_assertObject(token, "The specified token is null.");

    /* The text should not be null. */
    jtk_CString_delete(token->m_text);
    token->m_text = jtk_CString_newEx(text, length);
    token->m_length = length;
}

const uint8_t* k_Token_getText(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_text;
}


int32_t k_Token_getLength(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_length;
}


void k_Token_setStartIndex(k_Token_t* token, int32_t startIndex) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_startIndex = startIndex;
}

void k_Token_setStopIndex(k_Token_t* token, int32_t stopIndex) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_stopIndex = stopIndex;
}


void k_Token_setStartLine(k_Token_t* token, int32_t startLine) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_startLine = startLine;
}

int32_t k_Token_getStartLine(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_startLine;
}

void k_Token_setStopLine(k_Token_t* token, int32_t stopLine) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_startLine = stopLine;
}

int32_t k_Token_getStopLine(k_Token_t* token) {
    return token->m_stopLine;
}

void k_Token_setStartColumn(k_Token_t* token, int32_t startColumn) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_startColumn = startColumn;
}

int32_t k_Token_getStartColumn(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_startColumn;
}


void k_Token_setStopColumn(k_Token_t* token, int32_t stopColumn) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_stopColumn = stopColumn;
}

int32_t k_Token_getStopColumn(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_stopColumn;
}

void k_Token_setIndex(k_Token_t* token, int32_t index) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->m_index = index;
}

int32_t k_Token_getIndex(k_Token_t* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->m_index;
}


/*******************************************************************************
 * TokenType                                                                   *
 *******************************************************************************/

// Modifiers

uint32_t k_TokenType_toModifiers(k_TokenType_t type) {
    uint32_t modifiers = 0;
    switch (type) {
    case KUSH_TOKEN_KEYWORD_NATIVE: {
        modifiers |= KUSH_MODIFIER_NATIVE;
        break;
    }
    }
    return modifiers;
}

// Assignment Operator

bool k_TokenType_isAssignmentOperator(k_TokenType_t type) {
    return (type == KUSH_TOKEN_EQUAL) ||
           (type == KUSH_TOKEN_MODULUS_EQUAL) ||
           (type == KUSH_TOKEN_AMPERSAND_EQUAL) ||
           (type == KUSH_TOKEN_ASTERISK_2_EQUAL) ||
           (type == KUSH_TOKEN_ASTERISK_EQUAL) ||
           (type == KUSH_TOKEN_PLUS_EQUAL) ||
           (type == KUSH_TOKEN_DASH_EQUAL) ||
           (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == KUSH_TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_3_EQUAL) ||
           (type == KUSH_TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL) ||
           (type == KUSH_TOKEN_VERTICAL_BAR_EQUAL);
}
