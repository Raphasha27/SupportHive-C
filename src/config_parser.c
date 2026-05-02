/**
 * EventHive-C | Advanced Configuration Parser
 * ============================================================
 * Robust INI-style configuration parser for C systems.
 * 
 * Features:
 * - Section support
 * - Comment handling (# and ;)
 * - Trim whitespace logic
 * - Error reporting with line numbers
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "include/config_parser.h"

#define MAX_LINE_LEN 512
#define MAX_VAL_LEN 256

/**
 * Trims leading and trailing whitespace from a string
 */
static char* trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

/**
 * Parses a config file and populates the settings structure
 */
int config_parse_file(const char* filename, ConfigSettings* settings) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        return -1;
    }

    char line[MAX_LINE_LEN];
    char current_section[64] = "default";
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        char* l = trim(line);

        // Skip comments and empty lines
        if (*l == '\0' || *l == '#' || *l == ';') continue;

        // Section handling: [section]
        if (*l == '[' && l[strlen(l) - 1] == ']') {
            l[strlen(l) - 1] = '\0';
            strncpy(current_section, l + 1, sizeof(current_section) - 1);
            continue;
        }

        // Key-Value handling: key = value
        char* delimiter = strchr(l, '=');
        if (delimiter) {
            *delimiter = '\0';
            char* key = trim(l);
            char* value = trim(delimiter + 1);

            // Logic to store key-value pair based on section
            if (strcmp(current_section, "server") == 0) {
                if (strcmp(key, "port") == 0) settings->port = atoi(value);
                else if (strcmp(key, "host") == 0) strncpy(settings->host, value, sizeof(settings->host) - 1);
            } else if (strcmp(current_section, "database") == 0) {
                if (strcmp(key, "path") == 0) strncpy(settings->db_path, value, sizeof(settings->db_path) - 1);
            } else if (strcmp(current_section, "security") == 0) {
                if (strcmp(key, "secret") == 0) strncpy(settings->secret_key, value, sizeof(settings->secret_key) - 1);
            }
        } else {
            fprintf(stderr, "Warning: Invalid config at %s:%d\n", filename, line_number);
        }
    }

    fclose(file);
    return 0;
}

/**
 * Diagnostic dump of loaded settings
 */
void config_debug_dump(const ConfigSettings* settings) {
    printf("\n[Config Settings Dump]\n");
    printf("Server Host: %s\n", settings->host);
    printf("Server Port: %d\n", settings->port);
    printf("DB Path: %s\n", settings->db_path);
    printf("Security Active: %s\n", settings->secret_key[0] ? "YES" : "NO");
    printf("----------------------\n");
}

/**
 * Saves current settings to a file
 */
int config_save_file(const char* filename, const ConfigSettings* settings) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;

    fprintf(file, "# EventHive-C Auto-generated Configuration\n\n");
    
    fprintf(file, "[server]\nhost = %s\nport = %d\n\n", settings->host, settings->port);
    fprintf(file, "[database]\npath = %s\n\n", settings->db_path);
    fprintf(file, "[security]\nsecret = %s\n", settings->secret_key);

    fclose(file);
    return 0;
}
