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

// Monday, October 07, 2019

#ifndef COM_ONECUBE_ZEN_COMPILER_GENERATOR_CONSTANT_POOL_BUILDER_H
#define COM_ONECUBE_ZEN_COMPILER_GENERATOR_CONSTANT_POOL_BUILDER_H

#include <jtk/collection/list/ArrayList.h>

#include <kush/virtual-machine/feb/constant-pool/ConstantPoolClass.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolDouble.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolEntry.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolField.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolFloat.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolFunction.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolInteger.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolLong.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolString.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolTag.h>
#include <kush/virtual-machine/feb/constant-pool/ConstantPoolUtf8.h>

/*******************************************************************************
 * ConstantPoolBuilder                                                         *
 *******************************************************************************/

/**
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct k_ConstantPoolBuilder_t {
    jtk_ArrayList_t* m_entries;
};

/**
 * @memberof ConstantPoolBuilder
 */
typedef struct k_ConstantPoolBuilder_t k_ConstantPoolBuilder_t;

// Constructor

/**
 * @memberof ConstantPoolBuilder
 */
k_ConstantPoolBuilder_t* k_ConstantPoolBuilder_new();

// Destructor

/**
 * @memberof ConstantPoolBuilder
 */
void k_ConstantPoolBuilder_delete(k_ConstantPoolBuilder_t* builder);

// Clear

void k_ConstantPoolBuilder_clear(k_ConstantPoolBuilder_t* builder);

// Reset

void k_ConstantPoolBuilder_reset(k_ConstantPoolBuilder_t* builder);

// Entries

/**
 * @memberof ConstantPoolBuilder
 */
int32_t k_ConstantPoolBuilder_countEntries(k_ConstantPoolBuilder_t* builder);

/**
 * @memberof ConstantPoolBuilder
 */
k_ConstantPoolEntry_t* k_ConstantPoolBuilder_getEntry(k_ConstantPoolBuilder_t* builder,
    int32_t index);

// Class Entry

k_ConstantPoolClass_t* k_ConstantPoolBuilder_getClassEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getClassEntryIndexEx(
    k_ConstantPoolBuilder_t* builder, uint8_t* bytes, int32_t bytesSize);

// Field Entry

k_ConstantPoolField_t* k_ConstantPoolBuilder_getFieldEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getFieldEntryIndexEx(
    k_ConstantPoolBuilder_t* builder, const uint8_t* class0, int32_t classSize,
    const uint8_t* descriptor, int32_t descriptorSize, const uint8_t* name,
    int32_t nameSize);

// Function Entry

k_ConstantPoolFunction_t* k_ConstantPoolBuilder_getFunctionEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getFunctionEntryIndexEx(
    k_ConstantPoolBuilder_t* builder, const uint8_t* class0, int32_t classSize,
    const uint8_t* descriptor, int32_t descriptorSize, const uint8_t* name,
    int32_t nameSize, uint16_t tableIndex);

// Integer Entry

k_ConstantPoolInteger_t* k_ConstantPoolBuilder_getIntegerEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getIntegerEntryIndex(
    k_ConstantPoolBuilder_t* builder, int32_t value);

// String Entry

k_ConstantPoolString_t* k_ConstantPoolBuilder_getStringEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getStringEntryIndexEx(
    k_ConstantPoolBuilder_t* builder, uint8_t* bytes, int32_t bytesSize);

// UTF-8 Entry

k_ConstantPoolUtf8_t* k_ConstantPoolBuilder_getUtf8Entry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

// Long Entry

k_ConstantPoolLong_t* k_ConstantPoolBuilder_getLongEntry(
    k_ConstantPoolBuilder_t* builder, int32_t index);

int32_t k_ConstantPoolBuilder_getLongEntryIndex(
    k_ConstantPoolBuilder_t* builder, int64_t value);

int32_t k_ConstantPoolBuilder_getUtf8EntryIndexEx(
    k_ConstantPoolBuilder_t* builder, uint8_t* bytes, int32_t bytesSize);

#endif /* COM_ONECUBE_ZEN_COMPILER_GENERATOR_CONSTANT_POOL_BUILDER_H */