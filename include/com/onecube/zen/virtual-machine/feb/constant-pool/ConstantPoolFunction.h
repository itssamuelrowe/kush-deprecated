/*
 * Copyright 2018-2020 Samuel Rowe
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

// Saturday, April 28, 2018

#ifndef ZEN_FEB_CONSTANT_POOL_FUNCTION_CONSTANT_POOL_H
#define ZEN_FEB_CONSTANT_POOL_FUNCTION_CONSTANT_POOL_H

#include <com/onecube/zen/Configuration.h>

/*******************************************************************************
 * ConstantPoolFunction                                                        *
 *******************************************************************************/

/**
 * Represents a function reference.
 *
 * @author Samuel Rowe
 * @since  zen 1.0
 */
struct zen_ConstantPoolFunction_t {

    /**
     * The type of the constant pool entry.
     */
    uint8_t m_tag;

    /**
     * A valid index into the constant pool table. The entry at this index is
     * a {@code zen_ConstantPoolClass_t} entry which encloses this field.
     */
    uint16_t m_classIndex;

    /**
     * A valid index into the constant pool table. The entry at this index is
     * a {@code zen_ConstantPoolUtf8_t} entry which represents the descriptor
     * of this function.
     */
    uint16_t m_descriptorIndex;

    /**
     * A valid index into the constant pool table. The entry at this index is
     * a {@code zen_ConstantPoolUtf_t} entry which represents the name of this
     * function.
     */
    uint16_t m_nameIndex;

    /**
     * A valid index into the function table in the class whose member this
     * function is. The entry at this index is a `zen_FunctionEntity_t`.
     * Further, the value `0` is reserved and when in use, the virtual machine
     * searches all the functions based on the other details in this structure
     * to identify the function.
     */
    uint16_t m_tableIndex;
};

typedef struct zen_ConstantPoolFunction_t zen_ConstantPoolFunction_t;

#endif /* ZEN_FEB_CONSTANT_POOL_FUNCTION_CONSTANT_POOL_H */