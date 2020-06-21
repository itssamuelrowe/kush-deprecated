#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kush-runtime.h"






// Saturday, May 30 2020

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

static int32_t countFreeLists(k_Allocator_t* allocator);
static bool isSorted(k_Allocator_t* allocator);
static void coalesce(k_Allocator_t* allocator);
static void insertFreeList(k_Allocator_t* allocator, k_FreeList_t* freeList);
static void addPage(k_Allocator_t* allocator);
static k_FreeList_t* findChunk(k_Allocator_t* allocator, size_t size);
static size_t divide(size_t a, size_t b);
static void* allocateLarge(k_Allocator_t* allocator, size_t size);

int32_t countFreeLists(k_Allocator_t* allocator) {
    int result = 0;

    k_FreeList_t* current = allocator->freeList;
    while (current != NULL) {
        result++;
        current = current->next;
    }

    return result;
}

bool isSorted(k_Allocator_t* allocator) {
    k_FreeList_t* current = allocator->freeList;
    bool result = true;

    while (current != NULL && current->next != NULL) {
        if (current > current->next) {
            result = false;
            break;
        }
        current = current->next;
    }
    return result;
}

void coalesce(k_Allocator_t* allocator) {
    k_FreeList_t* current = allocator->freeList;
    while (current != NULL) {
        if ((void*)current + current->size == (void*)current->next) {
            current->size = current->size + current->next->size;
            current->next = current->next->next;
        }
        else {
            current = current->next;
        }
    }

    if (!isSorted(allocator)) {
        printf("[internal error] The free lists are unsorted.\n");
    }
}

void insertFreeList(k_Allocator_t* allocator, k_FreeList_t* freeList) {
    k_FreeList_t* current = allocator->freeList;
    if (current == NULL) {
        /* There is no free list. The specified free list is the first one. */
        allocator->freeList = freeList;
    }
    else if (freeList < current) {
        /* Insert the new free list at the head of the linked list. */
        freeList->next = current;
        allocator->freeList = freeList;
    }
    else {
        while (true) {
            /* We are either at the end of the list or the new list should be
             * inserted between the current and current's successor.
             */
            if((current->next == NULL) ||
               (freeList > current && freeList < current->next)) {
                freeList->next = current->next;
                current->next = freeList;
                break;
            }
            // increment
            current = current->next;
        }
    }
}

void addPage(k_Allocator_t* allocator) {
    void* address = mmap(NULL, K_PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if ((intptr_t)address == -1) {
        printf("[internal error] Failed to map a page.\n");
    }
    else {
        k_FreeList_t* freeList = (k_FreeList_t*)address;
        freeList->size = K_PAGE_SIZE;
        freeList->next = NULL;
        insertFreeList(allocator, freeList);
        allocator->statistics.pagesMapped++;
    }
}

k_FreeList_t* findChunk(k_Allocator_t* allocator, size_t size) {
    k_FreeList_t* current = allocator->freeList;
    k_FreeList_t* previous = NULL;
    k_FreeList_t* secondBest = NULL;
    size_t minSize = 5000;
    k_FreeList_t* bestChunk = NULL;

    while (current != NULL) {
        if ((current->size >= size) && (current->size < minSize)) {
            minSize = current->size;
            bestChunk = current;
            secondBest = previous;
        }
        previous = current;
        current = current->next;
    }

    k_FreeList_t* result = bestChunk;
    /* If we did not find a chunk large enough, add another page
     * and try again.
     */
    if (bestChunk == NULL) {
        addPage(allocator);
        result = findChunk(allocator, size);
    }
    else {
        /* Remove the chunk from the free list before it is returned. */
        if (secondBest != NULL) {
            secondBest->next = bestChunk->next;
        }
        else {
            allocator->freeList = bestChunk->next;
        }

        /* Evaluate the unused memory in the best chunk. If it's large enough,
         * return it back to the free list.
         */
        size_t excessAmount = bestChunk->size - size;
        if (excessAmount > sizeof(k_FreeList_t*) + sizeof(size_t)) {
            bestChunk->size = size;
            void* nextFreeAddress = (void*)bestChunk + size;
            k_FreeList_t* excess = (k_FreeList_t*)nextFreeAddress;
            excess->size = excessAmount;
            excess->next = NULL;
            insertFreeList(allocator, excess);
        }
    }
    return result;
}

size_t divide(size_t a, size_t b) {
    size_t result = a / b;
    if (result * b != a) {
        result++;
    }
    return result;
}

void* allocateLarge(k_Allocator_t* allocator, size_t size) {
    int pageCount = divide(size, K_PAGE_SIZE);

    /* Map enough pages for the large allocation. */
    void* address = mmap(NULL, pageCount * K_PAGE_SIZE,
        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    void* result = NULL;
    if ((intptr_t)address == -1) {
        printf("[internal error] Failed to map a large page.\n");
    }
    else {
        k_FreeList_t* newChunk = (k_FreeList_t*)address;
        newChunk->size = pageCount * K_PAGE_SIZE;
        newChunk->next = 0;

        allocator->statistics.pagesMapped += pageCount;

        result = address + sizeof (size_t);
    }
    return result;
}

#define OBJECT_HEADER_SIZE sizeof (size_t)

void k_Allocator_initialize(k_Allocator_t* allocator) {
    allocator->freeList = NULL;
    allocator->statistics.pagesMapped = 0;
    allocator->statistics.pagesUnmapped = 0;
    allocator->statistics.chunksAllocated = 0;
    allocator->statistics.chunksFreed = 0;
    allocator->statistics.freeLength = 0;
    allocator->firstObject = NULL;
}

void k_Allocator_destroy(k_Allocator_t* allocator) {
}

void* k_Allocator_allocate(k_Allocator_t* allocator, size_t size) {
    void* result = NULL;
    if (size > 0) {
        /* The chunk size requested does not include the header. Therefore,
         * we add the header size to the requested size to evaluate the
         * true size.
         */
        size += OBJECT_HEADER_SIZE;

        // TODO: Check for integer overflows!

        if (size > K_PAGE_SIZE) {
            result = allocateLarge(allocator, size);
        }
        else {
            uint8_t* address = (uint8_t*)findChunk(allocator, size);

            allocator->statistics.chunksAllocated++;

            result = address + OBJECT_HEADER_SIZE;

            k_Object_t* object = (k_Object_t*)result;
            object->header.marked = false;
            object->header.next = allocator->firstObject;
            allocator->firstObject = object;
        }
    }
    return result;
}

void k_Allocator_deallocate(k_Allocator_t* allocator, void* object) {
    if (object != NULL) {
        allocator->statistics.chunksFreed++;
        k_FreeList_t* chunk = (k_FreeList_t*)(object - OBJECT_HEADER_SIZE);

        chunk->next = NULL;
        if (chunk->size > K_PAGE_SIZE) {
            int32_t pages = divide(chunk->size, K_PAGE_SIZE);
            int result = munmap(chunk, chunk->size);
            if (result == -1) {
                printf("[internal error] Failed to unmap large page.\n");
            }
            else {
                allocator->statistics.pagesUnmapped += pages;
            }
        }
        else {
            insertFreeList(allocator, chunk);
        }
        coalesce(allocator);
    }
}















void kush_main();

void printStats(k_Runtime_t* runtime) {
	k_AllocatorStatistics_t* statistics = &runtime->allocator->statistics;
    printf("[Allocator Statistics]\n");
    printf("Pages Mapped -> %d\n", statistics->pagesMapped);
    printf("Pages Unmapped -> %d\n", statistics->pagesUnmapped);
    printf("Chunks Allocated -> %d\n", statistics->chunksAllocated);
    printf("Chunks Freed -> %d\n", statistics->chunksFreed);
    printf("Free Lists Count -> %d\n", statistics->freeLength);
}

void kush_GC_printStats(k_Runtime_t* runtime) {
    k_Runtime_pushStackFrame(runtime, "GC_printStats", 13, 0);

    printStats(runtime);

    k_Runtime_popStackFrame(runtime);
}

void kush_print_i(k_Runtime_t* runtime, int32_t i) {
    k_Runtime_pushStackFrame(runtime, "print_i", 7, 0);

    printf("%d", i);

    k_Runtime_popStackFrame(runtime);
}

void kush_print_s(k_Runtime_t* runtime, k_String_t* s) {
    k_Runtime_pushStackFrame(runtime, "print_s", 7, 0);

    printf("%.*s", s->size, s->value);

    k_Runtime_popStackFrame(runtime);
}

void kush_printStackTrace(k_Runtime_t* runtime) {
    k_Runtime_pushStackFrame(runtime, "printStackTrace", 15, 0);

    printf("[Stack Trace]\n");
    k_StackFrame_t* current = runtime->stackFrames;
    while (current != NULL) {
        printf("    %.*s()\n", current->functionName->size, current->functionName->value);
        current = current->next;
    }

    k_Runtime_popStackFrame(runtime);

}

void kush_collect(k_Runtime_t* runtime) {
    k_Runtime_pushStackFrame(runtime, "collect", 7, 0);

    collect(runtime);

    k_Runtime_popStackFrame(runtime);
}


/*******************************************************************************
 * Runtime                                                                     *
 *******************************************************************************/

void k_Runtime_initialize(k_Runtime_t* runtime, k_Allocator_t* allocator) {
    runtime->allocator = allocator;
    runtime->stackFrames = NULL;
    runtime->stackFrameCount = 0;
    runtime->trace = NULL;
    runtime->traceCount = 0;
    runtime->tracing = false;
}

// TODO: Does the allocate function return NULL when the size is 0?
k_StackFrame_t* k_Runtime_pushStackFrame(k_Runtime_t* runtime, const uint8_t* name,
    int32_t nameSize, int32_t pointerCount) {
    k_StackFrame_t* stackFrame = malloc(sizeof (k_StackFrame_t));
    stackFrame->pointers = malloc(sizeof (void*) * pointerCount);
    stackFrame->pointerCount = pointerCount;
    // stackFrame->functionName = makeString(runtime, name);
    stackFrame->next = runtime->stackFrames;

    runtime->stackFrames = stackFrame;
    runtime->stackFrameCount++;

    return stackFrame;
}

void k_Runtime_popStackFrame(k_Runtime_t* runtime) {
    if (runtime->stackFrames != NULL) {
        k_StackFrame_t* temporary = runtime->stackFrames;
        runtime->stackFrames = runtime->stackFrames->next;
        runtime->stackFrameCount--;

        // Should we destroy the functionName?
        free(temporary->pointers);
        free(temporary);
    }
}

k_String_t* makeString(k_Runtime_t* runtime, const char* sequence) {
    k_String_t* self = k_Allocator_allocate(runtime->allocator,
        sizeof (k_String_t));
    self->size = strlen(sequence);
    self->value = malloc(sizeof (uint8_t) * (self->size + 1));
    int32_t i;
    for (i = 0; i < self->size; i++) {
        self->value[i] = sequence[i];
    }
    self->value[i] = '\0';
    return self;
}

void markCallStack(k_Runtime_t* runtime) {
    int32_t count = 0;
    k_StackFrame_t* current = runtime->stackFrames;
    while (current != NULL) {
        int32_t i;
        for (i = 0; i < current->pointerCount; i++) {
            k_Object_t* object = (k_Object_t*)current->pointers[i];
            object->header.marked = true;
            count++;
        }
        current = current->next;
    }
    printf("Marked: %d\n", count);
}

void sweep(k_Runtime_t* runtime) {
    int32_t count = 0;
    k_Object_t* object = runtime->allocator->firstObject;
    k_Object_t* previous = NULL;
    while (object != NULL) {
        k_Object_t* next = object->header.next;
        if (!object->header.marked) {
            if (runtime->allocator->firstObject == object) {
                runtime->allocator->firstObject = next;
            }

            if (previous != NULL) {
                previous->header.next = next;
            }

            k_Allocator_deallocate(runtime->allocator, object);
            count++;
        }
        else {
            object->header.marked = false;
            previous = object;
        }
        object = next;
    }
    printf("Freed %d objects!\n", count);
}

void collect(k_Runtime_t* runtime) {
    markCallStack(runtime);
    sweep(runtime);
}

void k_Runtime_destroy(k_Runtime_t* runtime) {
}

int main() {
    k_Runtime_t runtime;
    k_Allocator_t allocator;
    k_Allocator_initialize(&allocator);
    k_Runtime_initialize(&runtime, &allocator);

    kush_main();

    collect(&runtime);
    puts("\n");
    printStats(&runtime);

    k_Runtime_destroy(&runtime);
    k_Allocator_destroy(&allocator);

    return 0;
}