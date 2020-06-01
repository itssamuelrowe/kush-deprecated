#ifndef KUSH_ABSTRACT_MODULE_H
#define KUSH_ABSTRACT_MODULE_H

/* The Abstract Module binary format describes the layout in which function and structure
 * declarations are stored in ".am" files.
 */ 

/**
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Module_t {
	/**
	 * This attribute should always be equal to 0x3AE031999.
	 */
	uint32_t m_magicNumber;
	uint16_t m_majorVersion;
	uint16_t m_minorVersion;
	uint16_t m_structureCount;
	k_Structure_t** m_structures;
	uint16_t m_functionCount;
	k_Function_t** m_functions;
	
};

typedef struct k_Module_t k_Module_t;

/**
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Structure_t {
	uint16_t m_flags;
	
	/**
	 * The size of the structure name.
	 */
	uint16_t m_nameSize;

	/**
	 * The name of the structure.
	 */
	uint8_t* m_name;
	
	/**
	 * The number of attributes that are members of the structure.
	 */
	uint16_t m_attributeCount;

	/**
	 * An array of integers that represent the length of each type name.
	 */
	uint16_t* m_attributeNameSizes;

	/**
	 * An array of strings that represent the type names of each attribute.
	 */
	uint8_t** m_attributeNames;
};

typedef struct k_Structure_t k_Structure_t;

/**
 * @author Samuel Rowe
 * @since Kush 0.1
 */
struct k_Function_t {
	uint16_t m_flags;
	uint16_t m_nameSize;
	uint8_t* m_name;
	uint16_t m_signatureSize;
	uint8_t* m_signature;
	/**
	 * The captures are used to evaluate the life expectancy of an object
	 * that is passed an argument to a function. This allows us to allocate
	 * object on stack when the die within the stack frame where they were
	 * declared.
	 *
	 * Since, only 8 bits are allocated to this attribute, the maxium
	 * number of parameters a function can receive is 255.
	 */
	uint8_t m_captureCount;
	uint8_t* m_captures;
};

typedef struct k_Function_t k_Function_t;

#endif /* KUSH_ABSTRACT_MODULE_H */
