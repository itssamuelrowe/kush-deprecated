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

// Saturday, April 28, 2018

#ifndef KUSH_COMPILER_GENERATOR_DATA_CHANNEL_H
#define KUSH_COMPILER_GENERATOR_DATA_CHANNEL_H

#include <kush/Configuration.h>

/*******************************************************************************
 * DataChannel                                                                 *
 *******************************************************************************/

/**
 * @class DataChannel
 * @ingroup k_compiler_generator
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_DataChannel_t {
    uint8_t* m_bytes;
    int32_t m_index;
    int32_t m_capacity;
    int32_t m_identifier;
};

/**
 * @memberof DataChannel
 */
typedef struct k_DataChannel_t k_DataChannel_t;

// Constructor

k_DataChannel_t* k_DataChannel_new(int32_t identifier);

// Destructor

void k_DataChannel_delete(k_DataChannel_t* channel);

// Append

void k_DataChannel_appendChannel(k_DataChannel_t* channel, k_DataChannel_t* other);
void k_DataChannel_appendByte(k_DataChannel_t* channel, uint8_t byte);
void k_DataChannel_appendBytes(k_DataChannel_t* channel, uint8_t* bytes, int32_t size);
void k_DataChannel_appendBytesRange(k_DataChannel_t* channel, uint8_t* bytes, int32_t size, int32_t startIndex, int32_t stopIndex);

// Bytes

uint8_t* k_DataChannel_getBytes(k_DataChannel_t* channel);

// Capacity

void k_DataChannel_requestCapacity(k_DataChannel_t* channel, int32_t capacity);

// Size

int32_t k_DataChannel_getSize(k_DataChannel_t* channel);

#endif /* KUSH_COMPILER_GENERATOR_DATA_CHANNEL_H */
