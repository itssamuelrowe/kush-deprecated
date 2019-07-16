// Saturday, April 28, 2018

#ifndef COM_ONECUBE_ZEN_COMPILER_GENERATOR_DATA_CHANNEL_H
#define COM_ONECUBE_ZEN_COMPILER_GENERATOR_DATA_CHANNEL_H

#include <com/onecube/zen/Configuration.h>

/*******************************************************************************
 * DataChannel                                                                 *
 *******************************************************************************/

/**
 * @class DataChannel
 * @ingroup zen_compiler_generator
 * @author Samuel Rowe
 * @since zen 1.0
 */
struct zen_DataChannel_t {
    uint8_t* m_bytes;
    int32_t m_index;
    int32_t m_capacity;
    int32_t m_identifier;
};

/**
 * @memberof DataChannel
 */
typedef struct zen_DataChannel_t zen_DataChannel_t;

// Constructor

zen_DataChannel_t* zen_DataChannel_new(int32_t identifier);

// Destructor

void zen_DataChannel_delete(zen_DataChannel_t* channel);

// Append

void zen_DataChannel_appendChannel(zen_DataChannel_t* channel, zen_DataChannel_t* other);
void zen_DataChannel_appendByte(zen_DataChannel_t* channel, uint8_t byte);
void zen_DataChannel_appendBytes(zen_DataChannel_t* channel, uint8_t* bytes, int32_t size);
void zen_DataChannel_appendBytesRange(zen_DataChannel_t* channel, uint8_t* bytes, int32_t size, int32_t startIndex, int32_t stopIndex);

// Capacity

void zen_DataChannel_requestCapacity(zen_DataChannel_t* channel, int32_t capacity);

// Size

int32_t zen_DataChannel_getSize(zen_DataChannel_t* channel);

#endif /* COM_ONECUBE_ZEN_COMPILER_GENERATOR_DATA_CHANNEL_H */