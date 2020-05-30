// Sunday, November 24, 2019

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

#include <kush/lexer/TokenType.h>
#include <kush/symbol-table/Modifier.h>

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
