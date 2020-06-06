// Monday, May 25 2020

#ifndef KUSH_H
#define KUSH_H

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * String                                                                     *
 ******************************************************************************/

/**
 * @author Samuel Rowe
 * @since 0.1
 */
struct k_String_t {
	uint8_t* value;
	int32_t length;
	uint32_t hashCode;
};

typedef k_String_t k_String_t;

#endif /* KUSH_H */