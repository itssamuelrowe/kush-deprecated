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

#ifndef COM_ONECUBE_ZEN_COMPILER_GENERATOR_BINARY_ENTITY_BUILDER_H
#define COM_ONECUBE_ZEN_COMPILER_GENERATOR_BINARY_ENTITY_BUILDER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/Configuration.h>
#include <kush/generator/DataChannel.h>

#include <kush/virtual-machine/feb/ByteCode.h>
#include <kush/virtual-machine/feb/attribute/ExceptionTable.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolEntry.h>

/*******************************************************************************
 * BinaryEntityBuilder                                                         *
 *******************************************************************************/

/**
 * The BinaryEntityBuilder exposes an interesting approach in assembling
 * binary entities. It consists of a stack of data channels. A data channel
 * has three states: active, inactive, and dead. The data channel at the
 * top of the stack, is the only active data channel.
 *
 * Whenever a new channel is pushed, it becomes the active channel making the
 * previous channel inactive. When an active data channel is popped, it moves
 * to the dead state. The data channel below the active channel becomes the next
 * active channel. When a data channel is in the dead state, the bytes of the
 * channel are appended to the bytes of the next active channel. After which,
 * the dead data channel is destroyed.
 *
 * The data channels allow the generators to lazily evaluate jump offsets,
 * header sizes, and many other values which would otherwise require complex
 * algorithms.
 *
 * @class BinaryEntityBuilder
 * @ingroup k_compiler_generator
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_BinaryEntityBuilder_t {
    jtk_ArrayList_t* m_channels;
    uint16_t m_constantPoolIndex;
    int32_t m_identifier;
    int32_t m_activeChannelIndex;
};

/**
 * @memberof BinaryEntityBuilder
 */
typedef struct k_BinaryEntityBuilder_t k_BinaryEntityBuilder_t;

// Constructor

/**
 * @memberof BinaryEntityBuilder
 */
k_BinaryEntityBuilder_t* k_BinaryEntityBuilder_new();

// Destructor

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_delete(k_BinaryEntityBuilder_t* builder);

// Channel

k_DataChannel_t* k_BinaryEntityBuilder_getChannel(k_BinaryEntityBuilder_t* builder,
    int32_t index);

int32_t k_BinaryEntityBuilder_pushChannel(k_BinaryEntityBuilder_t* builder);

void k_BinaryEntityBuilder_popChannel(k_BinaryEntityBuilder_t* builder);

k_DataChannel_t* k_BinaryEntityBuilder_getActiveChannel(k_BinaryEntityBuilder_t* builder);

void k_BinaryEntityBuilder_setActiveChannelIndex(k_BinaryEntityBuilder_t* builder,
    int32_t activeChannelIndex);

int32_t k_BinaryEntityBuilder_getActiveChannelIndex(k_BinaryEntityBuilder_t* builder);

// Emit

void k_BinaryEntityBuilder_emitByteCode(k_BinaryEntityBuilder_t* builder,
    k_ByteCode_t byteCode, ...);

// Print

void k_BinaryEntityBuilder_printChannel(k_BinaryEntityBuilder_t* builder);

void k_BinaryEntityBuilder_printChannels(k_BinaryEntityBuilder_t* builder);

// Magic Number

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeMagicNumber(k_BinaryEntityBuilder_t* builder);

// Version

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeMajorVersion(k_BinaryEntityBuilder_t* builder,
    uint16_t majorVersion);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeMinorVersion(k_BinaryEntityBuilder_t* builder,
    uint16_t minorVersion);

// Stream

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeStreamSize(k_BinaryEntityBuilder_t* builder,
    uint32_t streamSize);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeStreamFlags(k_BinaryEntityBuilder_t* builder,
    uint16_t streamFlags);

// Constant Pool

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeConstantPoolHeader(k_BinaryEntityBuilder_t* builder, int32_t entries);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolInteger(k_BinaryEntityBuilder_t* builder, uint32_t value);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolLong(k_BinaryEntityBuilder_t* builder, uint64_t value);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolLongEx(k_BinaryEntityBuilder_t* builder,
    uint32_t highBytes, uint32_t lowBytes);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolFloat(k_BinaryEntityBuilder_t* builder, float value);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolDouble(k_BinaryEntityBuilder_t* builder, double value);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolUtf8(k_BinaryEntityBuilder_t* builder, uint16_t length, uint8_t* value);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolString(k_BinaryEntityBuilder_t* builder, uint16_t stringIndex);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolFunction(k_BinaryEntityBuilder_t* builder, uint16_t classIndex, uint16_t descriptorIndex,
    uint16_t nameIndex, uint16_t tableIndex);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolField(k_BinaryEntityBuilder_t* builder, uint16_t classIndex,
    uint16_t descriptorIndex, uint16_t nameIndex);

/**
 * @memberof BinaryEntityBuilder
 */
uint16_t k_BinaryEntityBuilder_writeConstantPoolClass(k_BinaryEntityBuilder_t* builder, uint16_t nameIndex);

// Entity

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeEntityHeader(k_BinaryEntityBuilder_t* builder, uint8_t type, uint16_t flags,
    uint16_t reference);

// Class

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeClass(k_BinaryEntityBuilder_t* builder, uint16_t flags, uint16_t reference,
    uint16_t superclassCount, uint16_t* superclassIndexes);

// Attribute

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeAttributeCount(k_BinaryEntityBuilder_t* builder, uint16_t attributeCount);

// Field

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeFieldsHeader(k_BinaryEntityBuilder_t* builder,
    uint16_t fieldCount, uint16_t fieldTableSize);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeField(k_BinaryEntityBuilder_t* builder,
    uint16_t flags, uint16_t nameIndex, uint16_t descriptorIndex,
    uint16_t tableIndex);

// Function

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeFunctionsHeader(k_BinaryEntityBuilder_t* builder,
    uint16_t functionCount, uint16_t functionTableSize);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeFunction(k_BinaryEntityBuilder_t* builder,
    uint16_t flags, uint16_t nameIndex, uint16_t descriptorIndex,
    uint16_t tableIndex);

// Instruction Attribute

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_writeInstructionAttributeHeader(k_BinaryEntityBuilder_t* builder, uint16_t nameIndex,
    uint32_t length, uint16_t maxStackSize, uint16_t localVariableCount,
    uint32_t instructionCount);

void k_BinaryEntityBuilder_writeExceptionTableHeader(k_BinaryEntityBuilder_t* builder, uint16_t size);

// Instructions

/* NOP */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNop(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAddInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAddLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAddFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAddDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAndInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitAndLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitOrInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitOrLong(k_BinaryEntityBuilder_t* builder);

/* Shift Left */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftLeftInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftLeftLong(k_BinaryEntityBuilder_t* builder);

/* Shift Right */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftRightInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftRightLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftRightUnsignedInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitShiftRightUnsignedLong(k_BinaryEntityBuilder_t* builder);

/* XOR */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitXorInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitXorLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITB(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITS(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITL(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITF(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITD(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastLTB(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastLTS(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastLTI(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastLTF(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastLTD(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastFTI(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastFTL(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastFTD(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastDTI(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastDTL(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastDTF(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCastITC(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCheckCast(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCompareLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCompareLesserThanFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCompareGreaterThanFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCompareLesserThanDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitCompareGreaterThanDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDivideInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDivideLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDivideFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDivideDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicate(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicateX1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicateX2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicate2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicate2X1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitDuplicate2X2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpEqual0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpNotEqual0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpLesserThan0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpGreaterThan0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpLesserThanOrEqualTo0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpGreaterThanOrEqualTo0Integer(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpNotEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpLesserThanInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpGreaterThanInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpLesserThanOrEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpGreaterThanOrEqualInteger(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpEqualReference(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpNotEqualReference(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpEqualNullReference(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJumpNotEqualNullReference(k_BinaryEntityBuilder_t* builder, int16_t offset);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitIncrementInteger(k_BinaryEntityBuilder_t* builder, uint8_t index, uint8_t constant);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideIncrementInteger(k_BinaryEntityBuilder_t* builder, uint16_t index, uint16_t constant);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitInvokeSpecial(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitInvokeVirtual(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitInvokeDynamic(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitInvokeStatic(k_BinaryEntityBuilder_t* builder, uint16_t index);

/* Jump */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitJump(k_BinaryEntityBuilder_t* builder, uint16_t index);

/* Load */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadInteger(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadInteger(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadLong(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadLong(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadFloat(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadFloat(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadDouble(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadDouble(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadReference(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadReference(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad0Integer(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad1Integer(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad2Integer(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad3Integer(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad0Long(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad1Long(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad2Long(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad3Long(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad0Float(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad1Float(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad2Float(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad3Float(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad0Double(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad1Double(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad2Double(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad3Double(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad0Reference(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad1Reference(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad2Reference(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoad3Reference(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayByte(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayCharacter(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayShort(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArrayReference(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadInstanceField(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadStaticField(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadCPR(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideLoadCPR(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitLoadArraySize(k_BinaryEntityBuilder_t* builder);

/* Modulo */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitModuloInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitModuloLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitModuloFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitModuloDouble(k_BinaryEntityBuilder_t* builder);

/* Multiply */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitMultiplyInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitMultiplyLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitMultiplyFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitMultiplyDouble(k_BinaryEntityBuilder_t* builder);

/* Negate */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNegateInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNegateLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNegateFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNegateDouble(k_BinaryEntityBuilder_t* builder);

/* New */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNew(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNewArray(k_BinaryEntityBuilder_t* builder, uint8_t type);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNewReferenceArray(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitNewDimensionalArray(k_BinaryEntityBuilder_t* builder, uint16_t index,
    uint8_t dimensions);

/* Pop */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPop(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPop2(k_BinaryEntityBuilder_t* builder);

/* Push */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushNull(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushIntegerNegative1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger3(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger4(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushInteger5(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushLong0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushLong1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushLong2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushFloat0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushFloat1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushFloat2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushDouble0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushDouble1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushDouble2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushByte(k_BinaryEntityBuilder_t* builder, int8_t value);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitPushShort(k_BinaryEntityBuilder_t* builder, int16_t value);

/* Return */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturn(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturnInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturnLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturnFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturnDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitReturnReference(k_BinaryEntityBuilder_t* builder);

/* RTTI */

/* Store */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInteger(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideStoreInteger(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInteger0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInteger1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInteger2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInteger3(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreLong(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideStoreLong(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreLong0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreLong1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreLong2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreLong3(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreFloat(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideStoreFloat(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreFloat0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreFloat1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreFloat2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreFloat3(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreDouble(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideStoreDouble(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreDouble0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreDouble1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreDouble2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreDouble3(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreReference(k_BinaryEntityBuilder_t* builder, uint8_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWideStoreReference(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreReference0(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreReference1(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreReference2(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreReference3(k_BinaryEntityBuilder_t* builder);

/* Store Array */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayByte(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayCharacter(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayShort(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayDouble(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreArrayReference(k_BinaryEntityBuilder_t* builder);

/* Store Field */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreInstanceField(k_BinaryEntityBuilder_t* builder, uint16_t index);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitStoreStaticField(k_BinaryEntityBuilder_t* builder, uint16_t index);

/* Subtract */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitSubtractInteger(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitSubtractLong(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitSubtractFloat(k_BinaryEntityBuilder_t* builder);

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitSubtractDouble(k_BinaryEntityBuilder_t* builder);

/* Swap */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitSwap(k_BinaryEntityBuilder_t* builder);

/* Throw */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitThrow(k_BinaryEntityBuilder_t* builder);

/* Wide */

/**
 * @memberof BinaryEntityBuilder
 */
void k_BinaryEntityBuilder_emitWide(k_BinaryEntityBuilder_t* builder);

#endif /* COM_ONECUBE_ZEN_COMPILER_GENERATOR_BINARY_ENTITY_BUILDER_H */