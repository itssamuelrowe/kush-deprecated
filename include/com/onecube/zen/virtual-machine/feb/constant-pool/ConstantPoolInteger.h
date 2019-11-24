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

// Saturday, April 28, 2018

#ifndef ZEN_FEB_CONSTANT_POOL_INTEGER_CONSTANT_POOL_H
#define ZEN_FEB_CONSTANT_POOL_INTEGER_CONSTANT_POOL_H

#include <com/onecube/zen/Configuration.h>

/*******************************************************************************
 * ConstantPoolInteger                                                         *
 *******************************************************************************/

/**
 * Represents an integer reference.
 *
 * @author Samuel Rowe
 * @since  STK 1.0
 */
struct zen_ConstantPoolInteger_t {

    /**
The type of the constant pool entry.
     */
    uint8_t m_tag;

    /**
The value of the integer constant. The bytes are stored in the
big-endian order.
     */
    uint32_t m_bytes;
};

/**
 * @memberof ConstantPoolInteger
 */
typedef struct zen_ConstantPoolInteger_t zen_ConstantPoolInteger_t;

/* Value */

/**
 * @memberof ConstantPoolInteger
 */
int32_t zen_ConstantPoolInteger_getValue(zen_ConstantPoolInteger_t* constantPoolInteger);

#endif /* ZEN_FEB_CONSTANT_POOL_INTEGER_CONSTANT_POOL_H */