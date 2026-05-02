/**
 * EventHive-C | Advanced Memory Management
 * ============================================================
 * High-performance Memory Pool (Arena) Allocator
 * Designed for sub-millisecond event orchestration.
 * 
 * This module reduces fragmentation and allocation overhead by
 * pre-allocating large memory blocks and managing them in-process.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "include/memory_pool.h"

#define DEFAULT_POOL_SIZE (1024 * 1024) // 1MB

typedef struct MemoryBlock {
    void* data;
    size_t size;
    size_t used;
    struct MemoryBlock* next;
} MemoryBlock;

struct MemoryPool {
    MemoryBlock* head;
    MemoryBlock* current;
    size_t block_size;
    uint32_t total_allocations;
};

/**
 * Creates a new memory block
 */
static MemoryBlock* create_block(size_t size) {
    MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!block) return NULL;

    block->data = malloc(size);
    if (!block->data) {
        free(block);
        return NULL;
    }

    block->size = size;
    block->used = 0;
    block->next = NULL;
    return block;
}

/**
 * Initializes the memory pool
 */
MemoryPool* mem_pool_create(size_t initial_size) {
    MemoryPool* pool = (MemoryPool*)malloc(sizeof(MemoryPool));
    if (!pool) return NULL;

    size_t size = (initial_size > 0) ? initial_size : DEFAULT_POOL_SIZE;
    pool->head = create_block(size);
    if (!pool->head) {
        free(pool);
        return NULL;
    }

    pool->current = pool->head;
    pool->block_size = size;
    pool->total_allocations = 0;

    return pool;
}

/**
 * Allocates memory from the pool
 */
void* mem_pool_alloc(MemoryPool* pool, size_t size) {
    if (!pool || size == 0) return NULL;

    // Align size to 8 bytes for performance
    size = (size + 7) & ~7;

    // Check if current block has enough space
    if (pool->current->used + size > pool->current->size) {
        // Need a new block
        size_t next_size = (size > pool->block_size) ? size : pool->block_size;
        MemoryBlock* block = create_block(next_size);
        if (!block) return NULL;

        pool->current->next = block;
        pool->current = block;
    }

    void* ptr = (uint8_t*)pool->current->data + pool->current->used;
    pool->current->used += size;
    pool->total_allocations++;

    return ptr;
}

/**
 * Calloc implementation for the pool
 */
void* mem_pool_calloc(MemoryPool* pool, size_t count, size_t size) {
    size_t total = count * size;
    void* ptr = mem_pool_alloc(pool, total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

/**
 * Resets the pool without deallocating the underlying blocks
 * High performance way to reuse memory for short-lived event tasks
 */
void mem_pool_reset(MemoryPool* pool) {
    if (!pool) return;

    MemoryBlock* block = pool->head;
    while (block) {
        block->used = 0;
        block = block->next;
    }
    pool->current = pool->head;
    pool->total_allocations = 0;
}

/**
 * Destroys the pool and frees all memory
 */
void mem_pool_destroy(MemoryPool* pool) {
    if (!pool) return;

    MemoryBlock* block = pool->head;
    while (block) {
        MemoryBlock* next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    free(pool);
}

/**
 * Diagnostic info
 */
void mem_pool_debug(MemoryPool* pool) {
    if (!pool) return;

    printf("\n[Memory Pool Debug]\n");
    printf("Block Size: %zu bytes\n", pool->block_size);
    printf("Total Allocations: %u\n", pool->total_allocations);

    MemoryBlock* block = pool->head;
    int count = 0;
    while (block) {
        printf("Block %d: %zu/%zu used\n", count++, block->used, block->size);
        block = block->next;
    }
    printf("-------------------\n");
}
