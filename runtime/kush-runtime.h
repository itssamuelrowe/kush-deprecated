// Saturday, June 20 2020

#include <stdint.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



























/*******************************************************************************
 * StackFrame                                                                  *
 *******************************************************************************/

typedef struct k_StackFrame_t k_StackFrame_t;
typedef struct k_String_t k_String_t;

struct k_StackFrame_t {
    void** pointers;
    int32_t pointerCount;
    k_String_t* functionName;
    k_StackFrame_t* next;
};

/*******************************************************************************
 * Runtime                                                                     *
 *******************************************************************************/

typedef struct k_Allocator_t k_Allocator_t;

struct k_Runtime_t {
    k_Allocator_t* allocator;
    k_StackFrame_t* stackFrames;
    int32_t stackFrameCount;
    k_StackFrame_t* trace;
    int32_t traceCount;
    bool tracing;
};

typedef struct k_Runtime_t k_Runtime_t;

void k_Runtime_initialize(k_Runtime_t* runtime, k_Allocator_t* allocator);
void k_Runtime_destroy(k_Runtime_t* runtime);
k_StackFrame_t* k_Runtime_pushStackFrame(k_Runtime_t* runtime, const uint8_t* name,
    int32_t nameSize, int32_t pointerCount);
void k_Runtime_popStackFrame(k_Runtime_t* runtime);

typedef struct k_Object_t k_Object_t;

typedef struct k_ObjectHeader_t k_ObjectHeader_t;

struct k_ObjectHeader_t {
    bool marked;
    k_Object_t* next;
};

struct k_Object_t {
    k_ObjectHeader_t header;
};

/*******************************************************************************
 * String                                                                      *
 *******************************************************************************/

struct k_String_t {
    k_ObjectHeader_t header;
    int32_t size;
    uint8_t* value;
};

typedef struct k_String_t k_String_t;

k_String_t* makeString(k_Runtime_t* runtime, const char* sequence);
void collect(k_Runtime_t* runtime);

void kush_GC_printStats(k_Runtime_t* runtime);
void kush_printStackTrace(k_Runtime_t* runtime);
void kush_print_i(k_Runtime_t* runtime, int32_t n);
void kush_print_s(k_Runtime_t* runtime, k_String_t* s);
void kush_collect(k_Runtime_t* runtime);

#define K_PAGE_SIZE 4096

/******************************************************************************
 * AllocatorStatistics                                                        *
 ******************************************************************************/

struct k_AllocatorStatistics_t {
	int32_t pagesMapped;
	int32_t pagesUnmapped;
	int32_t chunksAllocated;
	int32_t chunksFreed;
	int32_t freeLength;
};

typedef struct k_AllocatorStatistics_t k_AllocatorStatistics_t;

/******************************************************************************
 * FreeList                                                                   *
 ******************************************************************************/

struct k_FreeList_t {
   size_t size;
   struct k_FreeList_t* next;
};

typedef struct k_FreeList_t k_FreeList_t;

/******************************************************************************
 * Allocator                                                                  *
 ******************************************************************************/

struct k_Allocator_t {
    k_AllocatorStatistics_t statistics;
    k_FreeList_t* freeList;
    k_Object_t* firstObject;
};

typedef struct k_Allocator_t k_Allocator_t;

void k_Allocator_initialize(k_Allocator_t* allocator);
void k_Allocator_destroy(k_Allocator_t* allocator);
void* k_Allocator_allocate(k_Allocator_t* allocator, size_t size);
void k_Allocator_deallocate(k_Allocator_t* allocator, void* object);



