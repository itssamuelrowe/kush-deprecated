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

// Monday, June 24, 2019

#ifndef COM_ONECUBE_ZEN_ASSEMBLER_LEXER_ASSEMBLY_TOKEN_H
#define COM_ONECUBE_ZEN_ASSEMBLER_LEXER_ASSEMBLY_TOKEN_H

/*******************************************************************************
 * AssemblyToken                                                               *
 *******************************************************************************/

enum zen_AssemblyToken_t {
    ZEN_ASSEMBLY_TOKEN_UNKNOWN,

    ZEN_ASSEMBLY_TOKEN_INDENTATION,
    ZEN_ASSEMBLY_TOKEN_DEDENTATION,

    ZEN_ASSEMBLY_TOKEN_WHITESPACE,
    ZEN_ASSEMBLY_TOKEN_NEWLINE,
    ZEN_ASSEMBLY_TOKEN_END_OF_STREAM,

    ZEN_ASSEMBLY_TOKEN_DOT,

    ZEN_ASSEMBLY_TOKEN_SINGLE_LINE_COMMENT,
    ZEN_ASSEMBLY_TOKEN_MULTILINE_COMMENT,

    ZEN_ASSEMBLY_TOKEN_IDENTIFIER,

    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NOP,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_ADD_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_ADD_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_ADD_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_ADD_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_AND_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_AND_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_OR_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_OR_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_LEFT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_LEFT_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_RIGHT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_RIGHT_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_RIGHT_UI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SHIFT_RIGHT_UL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_XOR_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_XOR_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITF,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_LTI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_LTF,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_LTD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_FTI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_FTL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_FTD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_DTI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_DTL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_DTF,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITB,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITC,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CAST_ITS,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_CHECK_CAST,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_COMPARE_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_COMPARE_LT_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_COMPARE_GT_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_COMPARE_LT_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_COMPARE_GT_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DIVIDE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DIVIDE_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DIVIDE_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DIVIDE_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE_X1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE_X2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE2_X1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_DUPLICATE2_X2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_EQ0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_NE0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_LT0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_GT0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_LE0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_GE0_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_EQ_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_NE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_LT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_GT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_LE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_GE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_EQ_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_NE_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_EQN_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP_NEN_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INCREMENT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INVOKE_SPECIAL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INVOKE_VIRTUAL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INVOKE_DYNAMIC,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INVOKE_STATIC,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_INVOKE_FRAGMENT,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_JUMP,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_I0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_I1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_I2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_I3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_L0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_L1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_L2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_L3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_F0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_F1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_F2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_F3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_D0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_D1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_D2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_D3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_A0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_A1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_A2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_A3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AB,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AC,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AS,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AF,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_AA,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_INSTANCE_FIELD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_STATIC_FIELD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_CPR,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_LOAD_ARRAY_SIZE,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MODULO_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MODULO_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MODULO_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MODULO_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MULTIPLY_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MULTIPLY_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MULTIPLY_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_MULTIPLY_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEGATE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEGATE_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEGATE_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEGATE_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEW,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEW_ARRAY,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEW_ARRAY_N,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_NEW_ARRAY_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_POP,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_POP2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_NULL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_IN1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I4,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_I5,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_L0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_L1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_L2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_F0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_F1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_F2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_D0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_D1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_D2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_B,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_PUSH_S,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RETURN_FRAGMENT,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_RTTI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_I0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_I1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_I2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_I3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_L0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_L1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_L2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_L3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_F0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_F1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_F2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_F3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_D0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_D1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_D2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_D3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_A,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_A0,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_A1,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_A2,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_A3,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AB,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AC,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AS,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AI,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AL,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AF,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_AA,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_INSTANCE_FIELD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_STORE_STATIC_FIELD,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SUBTRACT_I,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SUBTRACT_L,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SUBTRACT_F,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SUBTRACT_D,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SWAP,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SWITCH_TABLE,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_SWITCH_SEARCH,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_THROW,
    ZEN_ASSEMBLY_TOKEN_INSTRUCTION_WIDE,
    
    ZEN_ASSEMBLY_TOKEN_KEYWORD_CODE,
    ZEN_ASSEMBLY_TOKEN_KEYWORD_CONSTANT_POOL,
    ZEN_ASSEMBLY_TOKEN_KEYWORD_FUNCTION,
    
    ZEN_ASSEMBLY_TOKEN_INTEGER_LITERAL,
    ZEN_ASSEMBLY_TOKEN_STRING_LITERAL,
    ZEN_ASSEMBLY_TOKEN_FLOATING_POINT_LITERAL,
}

#endif /* COM_ONECUBE_ZEN_ASSEMBLER_LEXER_ASSEMBLY_TOKEN_H */