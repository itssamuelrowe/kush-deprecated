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

#include <kush/generator/DataChannel.h>

/*******************************************************************************
 * DataChannel                                                                 *
 *******************************************************************************/

#define ZEN_BYTE_CODE_CHANNEL_DEFAULT_CAPACITY 1024

// Constructor

k_DataChannel_t* k_DataChannel_new(int32_t identifier) {
    k_DataChannel_t* channel = k_Memory_allocate(k_DataChannel_t, 1);
    channel->m_bytes = k_Memory_allocate(uint8_t, ZEN_BYTE_CODE_CHANNEL_DEFAULT_CAPACITY);
    channel->m_capacity = ZEN_BYTE_CODE_CHANNEL_DEFAULT_CAPACITY;
    channel->m_index = 0;
    channel->m_identifier = identifier;

    return channel;
}

// Destructor

void k_DataChannel_delete(k_DataChannel_t* channel) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");

    jtk_Memory_deallocate(channel->m_bytes);
    jtk_Memory_deallocate(channel);
}

// Append

void k_DataChannel_appendChannel(k_DataChannel_t* channel,
    k_DataChannel_t* other) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");

    k_DataChannel_requestCapacity(channel, channel->m_index + other->m_index);
    int32_t i;
    for (i = 0; i < other->m_index; i++) {
        channel->m_bytes[channel->m_index + i] = other->m_bytes[i];
    }
    channel->m_index += other->m_index;
}

void k_DataChannel_appendByte(k_DataChannel_t* channel, uint8_t byte) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");

    k_DataChannel_requestCapacity(channel, channel->m_index + 1);
    channel->m_bytes[channel->m_index] = byte;
    channel->m_index++;
}

void k_DataChannel_appendBytes(k_DataChannel_t* channel,
    uint8_t* bytes, int32_t size) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");

    k_DataChannel_appendBytesRange(channel, bytes, size, 0, size);
}

void k_DataChannel_appendBytesRange(k_DataChannel_t* channel,
    uint8_t* bytes, int32_t size, int32_t startIndex, int32_t stopIndex) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");
    // jtk_Assert_assertTrue(...);

    int32_t byteCount = stopIndex - startIndex;
    k_DataChannel_requestCapacity(channel, channel->m_index + byteCount);
    int32_t i;
    int32_t j;
    for (i = channel->m_index, j = startIndex; j < stopIndex; i++, j++) {
        channel->m_bytes[i] = bytes[j];
    }
    channel->m_index += byteCount;
}

// Capacity

void k_DataChannel_requestCapacity(k_DataChannel_t* channel, int32_t capacity) {
    if (capacity > 0) {
        capacity = channel->m_index + capacity;
        int32_t currentCapacity = channel->m_capacity;
        int32_t minimumCapacity = channel->m_index + capacity;
        int32_t requireCapacity = minimumCapacity - currentCapacity;
        if (requireCapacity > 0) {
            int32_t newCapacity = (currentCapacity * 2) + 2;
            if ((newCapacity - capacity) < 0) {
                newCapacity = capacity;
            }
            if (newCapacity < 0) {
                jtk_Assert_assertFalse(capacity < 0, "An int32_t overflow occurred, the requested capacity is too big.");
                newCapacity = 0x7FFFFFFF;
            }
            uint8_t* temporary = channel->m_bytes;
            channel->m_bytes = jtk_Memory_allocate(uint8_t, newCapacity);
            int32_t i;
            for (i = 0; i < channel->m_index; i++) {
                channel->m_bytes[i] = temporary[i];
            }
            jtk_Memory_deallocate(temporary);
            channel->m_capacity = newCapacity;
        }
    }
}

// Size

int32_t k_DataChannel_getSize(k_DataChannel_t* channel) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");
    return channel->m_index;
}

// Buffer

uint8_t* k_DataChannel_getBytes(k_DataChannel_t* channel) {
    jtk_Assert_assertObject(channel, "The specified byte code channel is null.");
    return channel->m_bytes;
}