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

// Tuesday, July 16, 2019

#include <jtk/core/Double.h>
#include <jtk/core/Float.h>
#include <jtk/core/VariableArguments.h>

#include <kush/generator/BinaryEntityBuilder.h>
#include <com/onecube/k/virtual-machine/feb/ByteCode.h>
#include <com/onecube/k/virtual-machine/feb/EntityType.h>
#include <com/onecube/k/virtual-machine/feb/Instruction.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPool.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolClass.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolDouble.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolEntry.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolField.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolFloat.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolFunction.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolInteger.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolLong.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolString.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolTag.h>
#include <com/onecube/k/virtual-machine/feb/constant-pool/ConstantPoolUtf8.h>

/*******************************************************************************
 * BinaryEntityBuilder                                                         *
 *******************************************************************************/

// Constructor

k_BinaryEntityBuilder_t* k_BinaryEntityBuilder_new() {
    k_BinaryEntityBuilder_t* builder = jtk_Memory_allocate(k_BinaryEntityBuilder_t, 1);
    builder->m_channels = jtk_ArrayList_new();
    builder->m_constantPoolIndex = 0;
    builder->m_identifier = 0;
    builder->m_activeChannelIndex = -1;

    return builder;
}

// Destructor

void k_BinaryEntityBuilder_delete(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");

    jtk_Iterator_t* iterator = jtk_ArrayList_getIterator(builder->m_channels);
    while (jtk_Iterator_hasNext(iterator)) {
        k_DataChannel_t* channel = (k_DataChannel_t*)jtk_Iterator_getNext(iterator);
        if (channel != NULL) {
            k_DataChannel_delete(channel);
        }
    }
    jtk_Iterator_delete(iterator);

    jtk_ArrayList_delete(builder->m_channels);
    jtk_Memory_deallocate(builder);
}

// Channel

int32_t k_BinaryEntityBuilder_addChannel(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");

    int32_t result = jtk_ArrayList_getSize(builder->m_channels);

    k_DataChannel_t* channel = k_DataChannel_new(builder->m_identifier++);
    jtk_ArrayList_add(builder->m_channels, channel);

    return result;
}

void k_BinaryEntityBuilder_removeChannel(k_BinaryEntityBuilder_t* builder,
    int32_t index) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");

    // TODO: Add a return value to jtk_ArrayList_removeLast()
    k_DataChannel_t* deadChannel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels,
        index);
    // jtk_ArrayList_removeIndex(builder->m_channels, index);
    // jtk_ArrayList_setValue(builder->m_channels, index, NULL);
    jtk_ArrayList_removeIndex(builder->m_channels, index);

    /* if (!jtk_ArrayList_isEmpty(builder->m_channels)) {
        k_DataChannel_t* activeChannel = (k_DataChannel_t*)jtk_ArrayList_getValue(
            builder->m_channels, builder->m_primaryChannel);
        k_DataChannel_appendChannel(activeChannel, deadChannel);
    }
    */

    k_DataChannel_delete(deadChannel);
}

k_DataChannel_t* k_BinaryEntityBuilder_getChannel(k_BinaryEntityBuilder_t* builder,
    int32_t index) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");

    return (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, index);
}

k_DataChannel_t* k_BinaryEntityBuilder_getActiveChannel(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");

    /* NOTE: As of now, the active channel is located at zeroth index.
     * However, in the future this behavior may be subjected to change.
     */
    // return jtk_ArrayList_isEmpty(builder->m_channels) || (builder->m_activeChannelIndex < 0)?
    //    NULL : jtk_ArrayList_getValue(builder->m_channels, 0);

    return jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
}

void k_BinaryEntityBuilder_setActiveChannelIndex(k_BinaryEntityBuilder_t* builder,
    int32_t activeChannelIndex) {
    builder->m_activeChannelIndex = activeChannelIndex;
}

int32_t k_BinaryEntityBuilder_getActiveChannelIndex(k_BinaryEntityBuilder_t* builder) {
    return builder->m_activeChannelIndex;
}

// Emit

void k_BinaryEntityBuilder_emitByteCode(k_BinaryEntityBuilder_t* builder,
    k_ByteCode_t byteCode, ...) {
    jtk_Assert_assertObject(builder, "The specified binary entity builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);

    k_Instruction_t* instruction = k_Instruction_getInstance(byteCode);
    uint8_t argumentCount = k_Instruction_getArgumentCount(instruction);
    if (argumentCount > 0) {
        k_DataChannel_requestCapacity(channel, argumentCount + 1);

        jtk_VariableArguments_t variableArguments;
        jtk_VariableArguments_start(variableArguments, byteCode);

        channel->m_bytes[channel->m_index++] = (uint8_t)byteCode;
        int32_t i;
        for (i = 0; i < argumentCount; i++) {
            channel->m_bytes[channel->m_index++] =
                // (uint8_t)jtk_VariableArguments_argument(variableArguments, int32_t);
                (uint8_t)va_arg(variableArguments, int32_t);
        }

        jtk_VariableArguments_end(variableArguments);
    }
}

// Print

void k_BinaryEntityBuilder_printChannel(k_BinaryEntityBuilder_t* builder) {
}

void k_BinaryEntityBuilder_printChannels(k_BinaryEntityBuilder_t* builder) {
}

// Magic Number

void k_BinaryEntityBuilder_writeMagicNumber(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = 0xFE;
    channel->m_bytes[channel->m_index++] = 0xB7;
    channel->m_bytes[channel->m_index++] = 0x20;
    channel->m_bytes[channel->m_index++] = 0x00;
}

void k_BinaryEntityBuilder_writeMajorVersion(k_BinaryEntityBuilder_t* builder,
    uint16_t majorVersion) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = (majorVersion & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (majorVersion & 0x000000FF);
}

void k_BinaryEntityBuilder_writeMinorVersion(k_BinaryEntityBuilder_t* builder,
    uint16_t minorVersion) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = (minorVersion & 0x0000FF00) & 0x00;
    channel->m_bytes[channel->m_index++] = (minorVersion & 0x000000FF);
}

/*
void k_BinaryEntityBuilder_writeStreamSize(k_BinaryEntityBuilder_t* builder,
    uint32_t streamSize) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = (streamSize & 0xFF000000) >> 24; // stream size
    channel->m_bytes[channel->m_index++] = (streamSize & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (streamSize & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (streamSize & 0x000000FF);
}
*/

void k_BinaryEntityBuilder_writeStreamFlags(k_BinaryEntityBuilder_t* builder,
    uint16_t streamFlags) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = (streamFlags & 0x0000FF00) >> 8; // flags
    channel->m_bytes[channel->m_index++] = (streamFlags & 0x000000FF);
}

void k_BinaryEntityBuilder_writeConstantPoolHeader(k_BinaryEntityBuilder_t* builder, int32_t entries) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = (entries & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (entries & 0x000000FF);
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolInteger(k_BinaryEntityBuilder_t* builder, uint32_t value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_INTEGER; // Tag
    channel->m_bytes[channel->m_index++] = (value & 0xFF000000) >> 24;
    channel->m_bytes[channel->m_index++] = (value & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (value & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (value & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolLong(k_BinaryEntityBuilder_t* builder, uint64_t value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 9);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_LONG; // Tag
    channel->m_bytes[channel->m_index++] = (value & 0xFF00000000000000L) >> 56; // Value
    channel->m_bytes[channel->m_index++] = (value & 0x00FF000000000000L) >> 48;
    channel->m_bytes[channel->m_index++] = (value & 0x0000FF0000000000L) >> 40;
    channel->m_bytes[channel->m_index++] = (value & 0x000000FF00000000L) >> 32;
    channel->m_bytes[channel->m_index++] = (value & 0x00000000FF000000L) >> 24;
    channel->m_bytes[channel->m_index++] = (value & 0x0000000000FF0000L) >> 16;
    channel->m_bytes[channel->m_index++] = (value & 0x000000000000FF00L) >> 8;
    channel->m_bytes[channel->m_index++] = (value & 0x00000000000000FFL);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolLongEx(k_BinaryEntityBuilder_t* builder,
    uint32_t highBytes, uint32_t lowBytes) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_INTEGER; // Tag
    channel->m_bytes[channel->m_index++] = (highBytes & 0xFF000000) >> 24;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (lowBytes & 0xFF000000) >> 24;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolFloat(k_BinaryEntityBuilder_t* builder, float value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    uint32_t value0 = jtk_Float_unpack(value);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_FLOAT; // Tag
    channel->m_bytes[channel->m_index++] = (value0 & 0xFF000000) >> 24; // Value
    channel->m_bytes[channel->m_index++] = (value0 & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (value0 & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (value0 & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolFloatEx(k_BinaryEntityBuilder_t* builder,
    uint32_t value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_FLOAT; // Tag
    channel->m_bytes[channel->m_index++] = (value & 0xFF000000) >> 24; // Value
    channel->m_bytes[channel->m_index++] = (value & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (value & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (value & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolDouble(k_BinaryEntityBuilder_t* builder, double value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 9);

    uint64_t value0 = jtk_Double_unpack(value);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_DOUBLE; // Tag
    channel->m_bytes[channel->m_index++] = (value0 & 0xFF00000000000000L) >> 56; // Value
    channel->m_bytes[channel->m_index++] = (value0 & 0x00FF000000000000L) >> 48;
    channel->m_bytes[channel->m_index++] = (value0 & 0x0000FF0000000000L) >> 40;
    channel->m_bytes[channel->m_index++] = (value0 & 0x000000FF00000000L) >> 32;
    channel->m_bytes[channel->m_index++] = (value0 & 0x00000000FF000000L) >> 24;
    channel->m_bytes[channel->m_index++] = (value0 & 0x0000000000FF0000L) >> 16;
    channel->m_bytes[channel->m_index++] = (value0 & 0x000000000000FF00L) >> 8;
    channel->m_bytes[channel->m_index++] = (value0 & 0x00000000000000FFL);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolDoubleEx(k_BinaryEntityBuilder_t* builder,
    uint32_t highBytes, uint32_t lowBytes) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_INTEGER; // Tag
    channel->m_bytes[channel->m_index++] = (highBytes & 0xFF000000) >> 24;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (highBytes & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (lowBytes & 0xFF000000) >> 24;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (lowBytes & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolUtf8(k_BinaryEntityBuilder_t* builder, uint16_t length, uint8_t* value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3 + length);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_UTF8; // Tag
    channel->m_bytes[channel->m_index++] = (length & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (length & 0x000000FF);
    int32_t j = 0;
    while (j < length) {
        channel->m_bytes[channel->m_index + j] = value[j];
        j++;
    }
    channel->m_index += length;

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolString(k_BinaryEntityBuilder_t* builder, uint16_t stringIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_STRING; // Tag
    channel->m_bytes[channel->m_index++] = (stringIndex & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (stringIndex & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolFunction(
    k_BinaryEntityBuilder_t* builder, uint16_t classIndex, uint16_t descriptorIndex,
    uint16_t nameIndex, uint16_t tableIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 9);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_FUNCTION; // Tag
    channel->m_bytes[channel->m_index++] = (classIndex & 0x0000FF00) >> 8; // class index
    channel->m_bytes[channel->m_index++] = (classIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x0000FF00) >> 8; // descriptor index
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // name index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x0000FF00) >> 8; // table index
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolField(k_BinaryEntityBuilder_t* builder, uint16_t classIndex, uint16_t descriptorIndex,
    uint16_t nameIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 7);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_FIELD; // Tag
    channel->m_bytes[channel->m_index++] = (classIndex & 0x0000FF00) >> 8; // class index
    channel->m_bytes[channel->m_index++] = (classIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x0000FF00) >> 8; // descriptor index
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // name index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

uint16_t k_BinaryEntityBuilder_writeConstantPoolClass(k_BinaryEntityBuilder_t* builder, uint16_t nameIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_CONSTANT_POOL_TAG_CLASS; // Tag
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // name index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);

    return builder->m_constantPoolIndex++;
}

int32_t k_BinaryEntityBuilder_writeConstantPoolEntry(k_BinaryEntityBuilder_t* builder,
    k_ConstantPoolEntry_t* entry) {
    int32_t result = -1;
    switch (entry->m_tag) {
        case k_CONSTANT_POOL_TAG_INTEGER: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolInteger_t* constantPoolInteger =
                (k_ConstantPoolInteger_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolInteger(builder,
                constantPoolInteger->m_bytes);

            break;
        }

        case k_CONSTANT_POOL_TAG_LONG: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolLong_t* constantPoolLong =
                (k_ConstantPoolLong_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolLongEx(builder,
                constantPoolLong->m_highBytes, constantPoolLong->m_lowBytes);

            break;
        }

        case k_CONSTANT_POOL_TAG_FLOAT: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolFloat_t* constantPoolFloat =
                (k_ConstantPoolFloat_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolFloatEx(builder,
                constantPoolFloat->m_bytes);

            break;
        }

        case k_CONSTANT_POOL_TAG_DOUBLE: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolDouble_t* constantPoolDouble =
                (k_ConstantPoolDouble_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolDoubleEx(builder,
                constantPoolDouble->m_highBytes, constantPoolDouble->m_lowBytes);

            break;
        }

        case k_CONSTANT_POOL_TAG_UTF8: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolUtf8_t* constantPoolUtf8 =
                (k_ConstantPoolUtf8_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolUtf8(builder,
                constantPoolUtf8->m_length, constantPoolUtf8->m_bytes);

            break;
        }

        case k_CONSTANT_POOL_TAG_STRING: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolString_t* constantPoolString =
                (k_ConstantPoolString_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolString(builder,
                constantPoolString->m_stringIndex);

            break;
        }

        case k_CONSTANT_POOL_TAG_FUNCTION: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolFunction_t* constantPoolFunction =
                (k_ConstantPoolFunction_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolFunction(builder,
                constantPoolFunction->m_classIndex,
                constantPoolFunction->m_descriptorIndex,
                constantPoolFunction->m_nameIndex,
                constantPoolFunction->m_tableIndex);

            break;
        }

        case k_CONSTANT_POOL_TAG_FIELD: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolField_t* constantPoolField =
                (k_ConstantPoolField_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolField(builder,
                constantPoolField->m_classIndex,
                constantPoolField->m_descriptorIndex,
                constantPoolField->m_nameIndex);

            break;
        }

        case k_CONSTANT_POOL_TAG_CLASS: {
            /* Convert the constant pool entry to its specific type. */
            k_ConstantPoolClass_t* constantPoolClass =
                (k_ConstantPoolClass_t*)entry;
            /* Write the bytes of the constant pool entry to the data channel. */
            result = k_BinaryEntityBuilder_writeConstantPoolClass(builder,
                constantPoolClass->m_nameIndex);

            break;
        }

        default: {
            fprintf(stderr, "[internal error] Control should not reach here.\n");
        }
    }
    return result;
}

void k_BinaryEntityBuilder_writeEntityHeader(k_BinaryEntityBuilder_t* builder, uint8_t type, uint16_t flags,
    uint16_t reference) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 5);

    channel->m_bytes[channel->m_index++] = type; // type
    channel->m_bytes[channel->m_index++] = (flags & 0x0000FF00) >> 8; // flags
    channel->m_bytes[channel->m_index++] = (flags & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (reference & 0x0000FF00) >> 8; // reference
    channel->m_bytes[channel->m_index++] = (reference & 0x000000FF);
}

void k_BinaryEntityBuilder_writeSuperclasses(k_BinaryEntityBuilder_t* builder,
    uint16_t superclassCount, uint16_t* superclassIndexes) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 7 + (superclassCount * 2));

    channel->m_bytes[channel->m_index++] = (superclassCount & 0x0000FF00) >> 8; // superclass count
    channel->m_bytes[channel->m_index++] = (superclassCount & 0x000000FF);
    int32_t j;
    for (j = 0; j < superclassCount; j++) {
        uint16_t superclassIndex = superclassIndexes[j];
        channel->m_bytes[channel->m_index++] = (superclassIndex & 0x0000FF00) >> 8; // superclass index
        channel->m_bytes[channel->m_index++] = (superclassIndex & 0x000000FF);
    }
}

void k_BinaryEntityBuilder_writeAttributeCount(k_BinaryEntityBuilder_t* builder, uint16_t attributeCount) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = (attributeCount & 0x0000FF00) >> 8; // attribute count
    channel->m_bytes[channel->m_index++] = (attributeCount & 0x000000FF);
}

void k_BinaryEntityBuilder_writeFieldsHeader(k_BinaryEntityBuilder_t* builder,
    uint16_t fieldCount, uint16_t fieldTableSize) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = (fieldCount & 0x0000FF00) >> 8; // field count
    channel->m_bytes[channel->m_index++] = (fieldCount & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (fieldTableSize & 0x0000FF00) >> 8; // field table size
    channel->m_bytes[channel->m_index++] = (fieldTableSize & 0x000000FF);
}

void k_BinaryEntityBuilder_writeField(k_BinaryEntityBuilder_t* builder,
    uint16_t flags, uint16_t nameIndex, uint16_t descriptorIndex,
    uint16_t tableIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 8);

    channel->m_bytes[channel->m_index++] = (flags & 0x0000FF00) >> 8; // flags
    channel->m_bytes[channel->m_index++] = (flags & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // name index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x0000FF00) >> 8; // descriptor index
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x0000FF00) >> 8; // table index
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x000000FF);
}

void k_BinaryEntityBuilder_writeFunctionsHeader(k_BinaryEntityBuilder_t* builder,
    uint16_t functionCount, uint16_t functionTableSize) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = (functionCount & 0x0000FF00) >> 8; // function count
    channel->m_bytes[channel->m_index++] = (functionCount & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (functionTableSize & 0x0000FF00) >> 8; // function table size
    channel->m_bytes[channel->m_index++] = (functionTableSize & 0x000000FF);
}

void k_BinaryEntityBuilder_writeFunction(k_BinaryEntityBuilder_t* builder,
    uint16_t flags, uint16_t nameIndex, uint16_t descriptorIndex,
    uint16_t tableIndex) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 8);

    channel->m_bytes[channel->m_index++] = (flags & 0x0000FF00) >> 8; // flags
    channel->m_bytes[channel->m_index++] = (flags & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // name index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x0000FF00) >> 8; // descriptor index
    channel->m_bytes[channel->m_index++] = (descriptorIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x0000FF00) >> 8; // table index
    channel->m_bytes[channel->m_index++] = (tableIndex & 0x000000FF);
}

/* The localVariableCount and maxStackSize are in terms of 4-m_bytes. Therefore,
 * long, double and reference types each occupy a magnitude of 2.
 */
void k_BinaryEntityBuilder_writeInstructionAttributeHeader(
    k_BinaryEntityBuilder_t* builder, uint16_t nameIndex,
    uint32_t length, uint16_t maxStackSize, uint16_t localVariableCount,
    uint32_t instructionCount) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 14);

    channel->m_bytes[channel->m_index++] = (nameIndex & 0x0000FF00) >> 8; // Name Index
    channel->m_bytes[channel->m_index++] = (nameIndex & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (length & 0xFF000000) >> 24; // Length
    channel->m_bytes[channel->m_index++] = (length & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (length & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (length & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (maxStackSize & 0x0000FF00) >> 8; // Max Stack Size
    channel->m_bytes[channel->m_index++] = (maxStackSize & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (localVariableCount & 0x0000FF00) >> 8; // Local Variable Count
    channel->m_bytes[channel->m_index++] = (localVariableCount & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (instructionCount & 0xFF000000) >> 24; // Instruction Count
    channel->m_bytes[channel->m_index++] = (instructionCount & 0x00FF0000) >> 16;
    channel->m_bytes[channel->m_index++] = (instructionCount & 0x0000FF00) >> 8;
    channel->m_bytes[channel->m_index++] = (instructionCount & 0x000000FF);
}

void k_BinaryEntityBuilder_writeInstructionAttribute(
    k_BinaryEntityBuilder_t* builder, uint16_t nameIndex,
    uint32_t length, uint16_t maxStackSize, uint16_t localVariableCount,
    uint32_t instructionCount, uint8_t* instructions, k_ExceptionTable_t* exceptionTable) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_BinaryEntityBuilder_writeInstructionAttributeHeader(
        builder, nameIndex, length, maxStackSize, localVariableCount,
        instructionCount);

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(
        builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, instructionCount);

    jtk_Arrays_copyEx_b(instructions, instructionCount, 0, channel->m_bytes,
        channel->m_capacity, channel->m_index, instructionCount);
    channel->m_index += instructionCount;

    int32_t exceptionHandlerSiteCount = exceptionTable->m_size;
    k_DataChannel_requestCapacity(channel, 2 + (exceptionHandlerSiteCount * 8));

    channel->m_bytes[channel->m_index++] = (exceptionHandlerSiteCount & 0x0000FF00) >> 8; // Exception Table Size
    channel->m_bytes[channel->m_index++] = (exceptionHandlerSiteCount & 0x000000FF);

    int32_t exceptionHandlerSiteIndex;
    for (exceptionHandlerSiteIndex = 0;
        exceptionHandlerSiteIndex < exceptionHandlerSiteCount;
        exceptionHandlerSiteIndex++) {
        k_ExceptionHandlerSite_t* exceptionHandlerSite = (k_ExceptionHandlerSite_t*)
            exceptionTable->m_exceptionHandlerSites[exceptionHandlerSiteIndex];

        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_startIndex & 0x0000FF00) >> 8; // Start Index
        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_startIndex & 0x000000FF);

        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_stopIndex & 0x0000FF00) >> 8; // Stop Index
        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_stopIndex & 0x000000FF);

        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_handlerIndex & 0x0000FF00) >> 8; // Handler Index
        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_handlerIndex & 0x000000FF);

        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_exceptionClassIndex & 0x0000FF00) >> 8; // Exception Class Index
        channel->m_bytes[channel->m_index++] = (exceptionHandlerSite->m_exceptionClassIndex & 0x000000FF);
    }
}

void k_BinaryEntityBuilder_writeExceptionTableHeader(k_BinaryEntityBuilder_t* builder, uint16_t size) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = (size & 0x0000FF00) >> 8; // Size
    channel->m_bytes[channel->m_index++] = (size & 0x000000FF);
}

// Instructions

/* NOP */

void k_BinaryEntityBuilder_emitNop(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NOP;
}

void k_BinaryEntityBuilder_emitAddInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_ADD_I;
}

void k_BinaryEntityBuilder_emitAddLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_ADD_L;
}

void k_BinaryEntityBuilder_emitAddFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_ADD_F;
}

void k_BinaryEntityBuilder_emitAddDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_ADD_D;
}

void k_BinaryEntityBuilder_emitAndInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_AND_I;
}

void k_BinaryEntityBuilder_emitAndLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_AND_L;
}

void k_BinaryEntityBuilder_emitOrInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_OR_I;
}

void k_BinaryEntityBuilder_emitOrLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_OR_L;
}

/* Shift Left */

void k_BinaryEntityBuilder_emitShiftLeftInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_LEFT_I;
}

void k_BinaryEntityBuilder_emitShiftLeftLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_LEFT_L;
}

/* Shift Right */

void k_BinaryEntityBuilder_emitShiftRightInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_RIGHT_I;
}

void k_BinaryEntityBuilder_emitShiftRightLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_RIGHT_L;
}

void k_BinaryEntityBuilder_emitShiftRightUnsignedInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_RIGHT_UI;
}

void k_BinaryEntityBuilder_emitShiftRightUnsignedLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SHIFT_RIGHT_UL;
}

/* XOR */

void k_BinaryEntityBuilder_emitXorInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_XOR_I;
}

void k_BinaryEntityBuilder_emitXorLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_XOR_L;
}

void k_BinaryEntityBuilder_emitCastITB(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITB;
}

void k_BinaryEntityBuilder_emitCastITS(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITS;
}

void k_BinaryEntityBuilder_emitCastITL(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITL;
}

void k_BinaryEntityBuilder_emitCastITF(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITF;
}

void k_BinaryEntityBuilder_emitCastITD(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITD;
}

void k_BinaryEntityBuilder_emitCastLTB(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_LTB;
}

void k_BinaryEntityBuilder_emitCastLTS(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_LTS;
}

void k_BinaryEntityBuilder_emitCastLTI(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_LTI;
}

void k_BinaryEntityBuilder_emitCastLTF(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_LTF;
}

void k_BinaryEntityBuilder_emitCastLTD(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_LTD;
}

void k_BinaryEntityBuilder_emitCastFTI(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_FTI;
}

void k_BinaryEntityBuilder_emitCastFTL(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_FTL;
}

void k_BinaryEntityBuilder_emitCastFTD(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_FTD;
}

void k_BinaryEntityBuilder_emitCastDTI(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_DTI;
}

void k_BinaryEntityBuilder_emitCastDTL(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_DTL;
}

void k_BinaryEntityBuilder_emitCastDTF(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_DTF;
}

void k_BinaryEntityBuilder_emitCastITC(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CAST_ITC;
}

void k_BinaryEntityBuilder_emitCheckCast(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_CHECK_CAST;
}

void k_BinaryEntityBuilder_emitCompareLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_COMPARE_L;
}

void k_BinaryEntityBuilder_emitCompareLesserThanFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_COMPARE_LT_F;
}

void k_BinaryEntityBuilder_emitCompareGreaterThanFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_COMPARE_GT_F;
}

void k_BinaryEntityBuilder_emitCompareLesserThanDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_COMPARE_LT_D;
}

void k_BinaryEntityBuilder_emitCompareGreaterThanDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_COMPARE_GT_D;
}

void k_BinaryEntityBuilder_emitDivideInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DIVIDE_I;
}

void k_BinaryEntityBuilder_emitDivideLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DIVIDE_L;
}
void k_BinaryEntityBuilder_emitDivideFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DIVIDE_F;
}

void k_BinaryEntityBuilder_emitDivideDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DIVIDE_D;
}

void k_BinaryEntityBuilder_emitDuplicate(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE;
}

void k_BinaryEntityBuilder_emitDuplicateX1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE_X1;
}

void k_BinaryEntityBuilder_emitDuplicateX2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE_X2;
}

void k_BinaryEntityBuilder_emitDuplicate2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE2;
}

void k_BinaryEntityBuilder_emitDuplicate2X1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE2_X1;
}

void k_BinaryEntityBuilder_emitDuplicate2X2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_DUPLICATE2_X2;
}

void k_BinaryEntityBuilder_emitJumpEqual0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_EQ0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpNotEqual0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_NE0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpLesserThan0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_LT0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpGreaterThan0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_GT0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpLesserThanOrEqualTo0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_LE0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpGreaterThanOrEqualTo0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_GE0_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_EQ_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpNotEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_NE_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpLesserThanInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_LT_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpGreaterThanInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_GT_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpLesserThanOrEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_LE_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpGreaterThanOrEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_GE_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpEqualReference(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_EQ_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpNotEqualReference(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_NE_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpEqualNullReference(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_EQN_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitJumpNotEqualNullReference(k_BinaryEntityBuilder_t* builder, int16_t offset) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP_NEN_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (offset & 0x0000FF00) >> 8; // Offset
    channel->m_bytes[channel->m_index++] = (offset & 0x000000FF);
}

void k_BinaryEntityBuilder_emitIncrementInteger(k_BinaryEntityBuilder_t* builder, uint8_t index, uint8_t constant) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INCREMENT_I; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
    channel->m_bytes[channel->m_index++] = constant; // Constant
}

void k_BinaryEntityBuilder_emitWideIncrementInteger(k_BinaryEntityBuilder_t* builder, uint16_t index, uint16_t constant) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 6);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INCREMENT_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
    channel->m_bytes[channel->m_index++] = (constant & 0x0000FF00) >> 8; // Constant
    channel->m_bytes[channel->m_index++] = (constant & 0x000000FF);
}

void k_BinaryEntityBuilder_emitInvokeSpecial(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INVOKE_SPECIAL; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitInvokeVirtual(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INVOKE_VIRTUAL; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitInvokeDynamic(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INVOKE_DYNAMIC; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitInvokeStatic(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_INVOKE_STATIC; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

/* Jump */

void k_BinaryEntityBuilder_emitJump(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_JUMP; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

/* Load */

void k_BinaryEntityBuilder_emitLoadInteger(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I; // Byte Code
    channel->m_bytes[channel->m_index++] = index;
}

void k_BinaryEntityBuilder_emitWideLoadInteger(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadLong(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L; // Byte Code
    channel->m_bytes[channel->m_index++] = index;
}

void k_BinaryEntityBuilder_emitWideLoadLong(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadFloat(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F; // Byte Code
    channel->m_bytes[channel->m_index++] = index;
}

void k_BinaryEntityBuilder_emitWideLoadFloat(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadDouble(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D; // Byte Code
    channel->m_bytes[channel->m_index++] = index;
}

void k_BinaryEntityBuilder_emitWideLoadDouble(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadReference(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A; // Byte Code
    channel->m_bytes[channel->m_index++] = index;
}

void k_BinaryEntityBuilder_emitWideLoadReference(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoad0Integer(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I0; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad1Integer(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I1; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad2Integer(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I2; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad3Integer(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_I3; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad0Long(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L0; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad1Long(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L1; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad2Long(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L2; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad3Long(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_L3; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad0Float(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F0; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad1Float(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F1; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad2Float(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F2; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad3Float(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_F3; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad0Double(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D0; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad1Double(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D1; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad2Double(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D2; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad3Double(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_D3; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad0Reference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A0; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad1Reference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A1; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad2Reference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A2; // Byte Code
}

void k_BinaryEntityBuilder_emitLoad3Reference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_A3; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayByte(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AB; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayCharacter(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AC; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayShort(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AS; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AI; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AL; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AF; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AD; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadArrayReference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_AA; // Byte Code
}

void k_BinaryEntityBuilder_emitLoadInstanceField(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_INSTANCE_FIELD; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadStaticField(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_STATIC_FIELD; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadCPR(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_CPR; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideLoadCPR(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_CPR; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitLoadArraySize(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_LOAD_ARRAY_SIZE; // Byte Code
}

/* Modulo */

void k_BinaryEntityBuilder_emitModuloInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MODULO_I; // Byte Code
    k_DataChannel_requestCapacity(channel, 1);
}

void k_BinaryEntityBuilder_emitModuloLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MODULO_L; // Byte Code
}

void k_BinaryEntityBuilder_emitModuloFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MODULO_F; // Byte Code
}

void k_BinaryEntityBuilder_emitModuloDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MODULO_D; // Byte Code
}

/* Multiply */

void k_BinaryEntityBuilder_emitMultiplyInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MULTIPLY_I; // Byte Code
}

void k_BinaryEntityBuilder_emitMultiplyLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MULTIPLY_L; // Byte Code
}

void k_BinaryEntityBuilder_emitMultiplyFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MULTIPLY_F; // Byte Code
}

void k_BinaryEntityBuilder_emitMultiplyDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_MULTIPLY_D; // Byte Code
}

/* Negate */

void k_BinaryEntityBuilder_emitNegateInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEGATE_I; // Byte Code
}

void k_BinaryEntityBuilder_emitNegateLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEGATE_L; // Byte Code
}

void k_BinaryEntityBuilder_emitNegateFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEGATE_F; // Byte Code
}

void k_BinaryEntityBuilder_emitNegateDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEGATE_D; // Byte Code
}

/* New */

void k_BinaryEntityBuilder_emitNew(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEW; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitNewArray(k_BinaryEntityBuilder_t* builder, uint8_t type) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEW; // Byte Code
    channel->m_bytes[channel->m_index++] = (type & 0x0000FF00) >> 8; // Type
    channel->m_bytes[channel->m_index++] = (type & 0x000000FF);
}

void k_BinaryEntityBuilder_emitNewReferenceArray(k_BinaryEntityBuilder_t* builder,
    uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEW_ARRAY_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitNewDimensionalArray(k_BinaryEntityBuilder_t* builder, uint16_t index,
    uint8_t dimensions) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 4);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_NEW_ARRAY_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
    channel->m_bytes[channel->m_index++] = dimensions; // Dimensions
}

/* Pop */

void k_BinaryEntityBuilder_emitPop(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_POP; // Byte Code
}

void k_BinaryEntityBuilder_emitPop2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_POP2; // Byte Code
}

/* Push */

void k_BinaryEntityBuilder_emitPushNull(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_NULL; // Byte Code
}

void k_BinaryEntityBuilder_emitPushIntegerNegative1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_IN1; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I0; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I1; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I2; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I3; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger4(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I4; // Byte Code
}

void k_BinaryEntityBuilder_emitPushInteger5(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_I5; // Byte Code
}

void k_BinaryEntityBuilder_emitPushLong0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_L0; // Byte Code
}

void k_BinaryEntityBuilder_emitPushLong1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_L1; // Byte Code
}

void k_BinaryEntityBuilder_emitPushLong2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_L2; // Byte Code
}

void k_BinaryEntityBuilder_emitPushFloat0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_F0; // Byte Code
}

void k_BinaryEntityBuilder_emitPushFloat1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_F1; // Byte Code
}

void k_BinaryEntityBuilder_emitPushFloat2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_F2; // Byte Code
}

void k_BinaryEntityBuilder_emitPushDouble0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_D0; // Byte Code
}

void k_BinaryEntityBuilder_emitPushDouble1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_D1; // Byte Code
}

void k_BinaryEntityBuilder_emitPushDouble2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_D2; // Byte Code
}

void k_BinaryEntityBuilder_emitPushByte(k_BinaryEntityBuilder_t* builder, int8_t value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_B; // Byte Code
    channel->m_bytes[channel->m_index++] = value;
}

void k_BinaryEntityBuilder_emitPushShort(k_BinaryEntityBuilder_t* builder, int16_t value) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_PUSH_S; // Byte Code
    channel->m_bytes[channel->m_index++] = (value & 0x0000FF00) >> 8; // Value
    channel->m_bytes[channel->m_index++] = (value & 0x000000FF);
}

/* Return */

void k_BinaryEntityBuilder_emitReturn(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN; // Byte Code
}

void k_BinaryEntityBuilder_emitReturnInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN_I; // Byte Code
}

void k_BinaryEntityBuilder_emitReturnLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN_L; // Byte Code
}

void k_BinaryEntityBuilder_emitReturnFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN_F; // Byte Code
}

void k_BinaryEntityBuilder_emitReturnDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN_D; // Byte Code
}

void k_BinaryEntityBuilder_emitReturnReference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_RETURN_A; // Byte Code
}

/* RTTI */

/* Store */

void k_BinaryEntityBuilder_emitStoreInteger(k_BinaryEntityBuilder_t* builder,
    uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideStoreInteger(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreInteger0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I0; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreInteger1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I1; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreInteger2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I2; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreInteger3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_I3; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreLong(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideStoreLong(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreLong0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L0; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreLong1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L1; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreLong2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L2; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreLong3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_L3; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreFloat(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideStoreFloat(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreFloat0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F0; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreFloat1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F1; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreFloat2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F2; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreFloat3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_F3; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreDouble(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideStoreDouble(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreDouble0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D0; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreDouble1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D1; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreDouble2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D2; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreDouble3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_D3; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreReference(k_BinaryEntityBuilder_t* builder, uint8_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 2);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A; // Byte Code
    channel->m_bytes[channel->m_index++] = index; // Index
}

void k_BinaryEntityBuilder_emitWideStoreReference(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreReference0(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A0; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreReference1(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A1; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreReference2(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A2; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreReference3(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_A3; // Byte Code
}

/* Store Array */

void k_BinaryEntityBuilder_emitStoreArrayByte(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AB; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayCharacter(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AC; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayShort(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AS; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AI; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AL; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AF; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AD; // Byte Code
}

void k_BinaryEntityBuilder_emitStoreArrayReference(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_AA; // Byte Code
}

/* Store Field */

void k_BinaryEntityBuilder_emitStoreInstanceField(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_INSTANCE_FIELD; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

void k_BinaryEntityBuilder_emitStoreStaticField(k_BinaryEntityBuilder_t* builder, uint16_t index) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 3);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_STORE_STATIC_FIELD; // Byte Code
    channel->m_bytes[channel->m_index++] = (index & 0x0000FF00) >> 8; // Index
    channel->m_bytes[channel->m_index++] = (index & 0x000000FF);
}

/* Subtract */

void k_BinaryEntityBuilder_emitSubtractInteger(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SUBTRACT_I; // Byte Code
}

void k_BinaryEntityBuilder_emitSubtractLong(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SUBTRACT_L; // Byte Code
}

void k_BinaryEntityBuilder_emitSubtractFloat(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SUBTRACT_F; // Byte Code
}

void k_BinaryEntityBuilder_emitSubtractDouble(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SUBTRACT_D; // Byte Code
}

/* Swap */

void k_BinaryEntityBuilder_emitSwap(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_SWAP; // Byte Code
}

/* Throw */

void k_BinaryEntityBuilder_emitThrow(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_THROW; // Byte Code
}

/* Wide */

void k_BinaryEntityBuilder_emitWide(k_BinaryEntityBuilder_t* builder) {
    jtk_Assert_assertObject(builder, "The specified builder is null.");
    jtk_Assert_assertTrue(builder->m_activeChannelIndex >= 0, "Please activate a channel before emitting instructions.");

    k_DataChannel_t* channel = (k_DataChannel_t*)jtk_ArrayList_getValue(builder->m_channels, builder->m_activeChannelIndex);
    k_DataChannel_requestCapacity(channel, 1);

    channel->m_bytes[channel->m_index++] = k_BYTE_CODE_WIDE; // Byte Code
}
