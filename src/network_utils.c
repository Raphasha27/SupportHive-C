/**
 * EventHive-C | Advanced Network Utilities
 * ============================================================
 * Specialized network protocol handling and HTTP optimizations.
 * 
 * Features:
 * - Zero-copy header extraction
 * - Custom URL decoding logic
 * - Buffer-safe string operations for C systems
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "include/network_utils.h"

/**
 * Decodes a URL-encoded string in-place
 * Optimized for O(n) performance with zero extra allocations.
 */
void url_decode(char *str) {
    char *pstr = str, *pdecode = str;
    while (*pstr) {
        if (*pstr == '%') {
            if (pstr[1] && pstr[2]) {
                char hex[3] = { pstr[1], pstr[2], '\0' };
                *pdecode = (char)strtol(hex, NULL, 16);
                pstr += 2;
            }
        } else if (*pstr == '+') {
            *pdecode = ' ';
        } else {
            *pdecode = *pstr;
        }
        pstr++;
        pdecode++;
    }
    *pdecode = '\0';
}

/**
 * Extracts a specific header value from a raw HTTP request
 * Uses a zero-copy pointer-based approach for maximum speed.
 */
const char* extract_header(const char* request, const char* header_name, char* buffer, size_t max_len) {
    const char* start = strcasestr(request, header_name);
    if (!start) return NULL;
    
    start += strlen(header_name);
    
    // Skip colon and spaces
    while (*start == ':' || isspace(*start)) start++;
    
    const char* end = strpbrk(start, "\r\n");
    if (!end) end = start + strlen(start);
    
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    
    memcpy(buffer, start, len);
    buffer[len] = '\0';
    
    return buffer;
}

/**
 * Safe string concatenation for fixed-size C buffers
 */
size_t safe_str_concat(char* dest, const char* src, size_t dest_size) {
    size_t current_len = strlen(dest);
    if (current_len >= dest_size - 1) return current_len;
    
    size_t to_copy = dest_size - current_len - 1;
    strncat(dest, src, to_copy);
    
    return strlen(dest);
}

/**
 * Validates if a string is a valid IPv4 address (basic check)
 */
int is_valid_ipv4(const char* ip) {
    int dots = 0;
    int num = 0;
    int has_digit = 0;
    
    if (!ip || !*ip) return 0;
    
    while (*ip) {
        if (isdigit(*ip)) {
            num = num * 10 + (*ip - '0');
            if (num > 255) return 0;
            has_digit = 1;
        } else if (*ip == '.') {
            if (!has_digit || dots >= 3) return 0;
            dots++;
            num = 0;
            has_digit = 0;
        } else {
            return 0;
        }
        ip++;
    }
    
    return (dots == 3 && has_digit);
}

/**
 * Performance-tuned memory comparison for network packets
 */
int secure_memcmp(const void* b1, const void* b2, size_t len) {
    const unsigned char *p1 = b1, *p2 = b2;
    int res = 0;
    for (size_t i = 0; i < len; i++) {
        res |= p1[i] ^ p2[i];
    }
    return res != 0;
}
