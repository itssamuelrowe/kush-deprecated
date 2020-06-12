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
#include <kush/token.h>

Token* newToken(
    TokenChannel channel,
    TokenType type,
    const uint8_t* text,
    int32_t length,
    int32_t startIndex,
    int32_t stopIndex,
    int32_t startLine,
    int32_t stopLine,
    int32_t startColumn,
    int32_t stopColumn,
    const char* file) {
    Token* token = allocate(Token, 1);
    token->channel = channel;
    token->type = type;
    token->text = jtk_CString_newEx(text, length);
    token->length = length; // This is the length of the text representation!
    token->startIndex = startIndex;
    token->stopIndex = stopIndex;
    token->startLine = startLine;
    token->stopLine = stopLine;
    token->startColumn = startColumn;
    token->stopColumn = stopColumn;
    token->file = file;

    return token;
}

void deleteToken(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    jtk_CString_delete(token->text);
    deallocate(token);
}

TokenChannel k_Token_getChannel(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->channel;
}

void k_Token_setChannel(Token* token, TokenChannel channel) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->channel = channel;
}

TokenType k_Token_getType(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->type;
}

void k_Token_setType(Token* token, TokenType type) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->type = type;
}


void k_Token_setText(Token* token, const uint8_t* text, int32_t length) {
    jtk_Assert_assertObject(token, "The specified token is null.");

    /* The text should not be null. */
    jtk_CString_delete(token->text);
    token->text = jtk_CString_newEx(text, length);
    token->length = length;
}

const uint8_t* k_Token_getText(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->text;
}


int32_t k_Token_getLength(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->length;
}


void k_Token_setStartIndex(Token* token, int32_t startIndex) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->startIndex = startIndex;
}

void k_Token_setStopIndex(Token* token, int32_t stopIndex) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->stopIndex = stopIndex;
}


void k_Token_setStartLine(Token* token, int32_t startLine) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->startLine = startLine;
}

int32_t k_Token_getStartLine(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->startLine;
}

void k_Token_setStopLine(Token* token, int32_t stopLine) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->startLine = stopLine;
}

int32_t k_Token_getStopLine(Token* token) {
    return token->stopLine;
}

void k_Token_setStartColumn(Token* token, int32_t startColumn) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->startColumn = startColumn;
}

int32_t k_Token_getStartColumn(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->startColumn;
}


void k_Token_setStopColumn(Token* token, int32_t stopColumn) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->stopColumn = stopColumn;
}

int32_t k_Token_getStopColumn(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->stopColumn;
}

void k_Token_setIndex(Token* token, int32_t index) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    token->index = index;
}

int32_t k_Token_getIndex(Token* token) {
    jtk_Assert_assertObject(token, "The specified token is null.");
    return token->index;
}


/*******************************************************************************
 * TokenType                                                                   *
 *******************************************************************************/

// Modifiers

uint32_t k_TokenType_toModifiers(TokenType type) {
    uint32_t modifiers = 0;
    switch (type) {
    case TOKEN_KEYWORD_NATIVE: {
        // modifiers |= KUSH_MODIFIER_NATIVE;
        break;
    }
    }
    return modifiers;
}

// Assignment Operator

bool k_TokenType_isAssignmentOperator(TokenType type) {
    return (type == TOKEN_EQUAL) ||
           (type == TOKEN_MODULUS_EQUAL) ||
           (type == TOKEN_AMPERSAND_EQUAL) ||
           (type == TOKEN_ASTERISK_2_EQUAL) ||
           (type == TOKEN_ASTERISK_EQUAL) ||
           (type == TOKEN_PLUS_EQUAL) ||
           (type == TOKEN_DASH_EQUAL) ||
           (type == TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == TOKEN_LEFT_ANGLE_BRACKET_2_EQUAL) ||
           (type == TOKEN_RIGHT_ANGLE_BRACKET_2_EQUAL) ||
           (type == TOKEN_VERTICAL_BAR_EQUAL);
}