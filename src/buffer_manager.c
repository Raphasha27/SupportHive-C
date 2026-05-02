/**
 * EventHive-C | Advanced Buffer Management
 * ============================================================
 * Specialized buffer utilities for network I/O and protocol parsing.
 * 
 * Features:
 * - Dynamic buffer resizing
 * - Segment-based reading/writing
 * - Safety-focused memory boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "include/buffer.h"

#define INITIAL_BUFFER_CAPACITY 4096

typedef struct {
    uint8_t* data;
    size_t size;
    size_t capacity;
    size_t read_pos;
} IOBuffer;

/**
 * Creates a new dynamic buffer
 */
IOBuffer* buffer_create(size_t capacity) {
    IOBuffer* buf = (IOBuffer*)malloc(sizeof(IOBuffer));
    if (!buf) return NULL;

    size_t cap = (capacity > 0) ? capacity : INITIAL_BUFFER_CAPACITY;
    buf->data = (uint8_t*)malloc(cap);
    if (!buf->data) {
        free(buf);
        return NULL;
    }

    buf->size = 0;
    buf->capacity = cap;
    buf->read_pos = 0;
    return buf;
}

/**
 * Ensures the buffer has enough capacity
 */
static int buffer_ensure_capacity(IOBuffer* buf, size_t needed) {
    if (buf->size + needed <= buf->capacity) return 0;

    size_t new_cap = buf->capacity * 2;
    while (buf->size + needed > new_cap) new_cap *= 2;

    uint8_t* new_data = (uint8_t*)realloc(buf->data, new_cap);
    if (!new_data) return -1;

    buf->data = new_data;
    buf->capacity = new_cap;
    return 0;
}

/**
 * Writes data to the buffer
 */
int buffer_write(IOBuffer* buf, const void* data, size_t len) {
    if (buffer_ensure_capacity(buf, len) < 0) return -1;

    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    return 0;
}

/**
 * Reads data from the buffer
 */
size_t buffer_read(IOBuffer* buf, void* dest, size_t len) {
    size_t available = buf->size - buf->read_pos;
    size_t to_read = (len > available) ? available : len;

    if (to_read > 0) {
        memcpy(dest, buf->data + buf->read_pos, to_read);
        buf->read_pos += to_read;
    }

    // Compaction: if we've read more than half the buffer, move data to the start
    if (buf->read_pos > buf->capacity / 2) {
        size_t remaining = buf->size - buf->read_pos;
        if (remaining > 0) {
            memmove(buf->data, buf->data + buf->read_pos, remaining);
        }
        buf->size = remaining;
        buf->read_pos = 0;
    }

    return to_read;
}

/**
 * Resets the buffer positions
 */
void buffer_clear(IOBuffer* buf) {
    if (buf) {
        buf->size = 0;
        buf->read_pos = 0;
    }
}

/**
 * Frees the buffer and its data
 */
void buffer_destroy(IOBuffer* buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

/**
 * Returns a pointer to the current data segment
 */
uint8_t* buffer_get_raw(IOBuffer* buf, size_t* out_len) {
    if (!buf) return NULL;
    if (out_len) *out_len = buf->size - buf->read_pos;
    return buf->data + buf->read_pos;
}
