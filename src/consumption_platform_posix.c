/**
 * @file consumption_platform_posix.c
 * @brief POSIX platform implementation for Consumption Counter Module
 *
 * Example implementation for Linux/macOS/Unix-like systems.
 * Demonstrates how to implement platform abstractions.
 */

#include "consumption_platform.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <curl/curl.h>

/* Configuration */
#define STORAGE_FILE "consumption_data.bin"
#define LOG_FILE "consumption.log"

/* Global state */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static CURL* g_curl = NULL;

/* ============================================================================
 * TIME FUNCTIONS
 * ============================================================================ */

uint32_t consumption_platform_get_timestamp(void) {
    return (uint32_t)time(NULL);
}

/* ============================================================================
 * PERSISTENT STORAGE
 * ============================================================================ */

bool consumption_platform_storage_read(void* data, size_t size) {
    int fd = open(STORAGE_FILE, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    ssize_t bytes_read = read(fd, data, size);
    close(fd);

    return (bytes_read == (ssize_t)size);
}

bool consumption_platform_storage_write(const void* data, size_t size) {
    int fd = open(STORAGE_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return false;
    }

    ssize_t bytes_written = write(fd, data, size);
    close(fd);

    return (bytes_written == (ssize_t)size);
}

/* ============================================================================
 * NETWORKING
 * ============================================================================ */

bool consumption_platform_network_send(const char* endpoint,
                                     const char* data,
                                     size_t data_len) {
#ifdef USE_CURL
    if (!g_curl) {
        return false;
    }

    CURLcode res;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(g_curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(g_curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(g_curl, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(g_curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(g_curl, CURLOPT_TIMEOUT, 10L); /* 10 second timeout */

    /* Disable SSL verification for development (enable in production!) */
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(g_curl);

    curl_slist_free_all(headers);

    return (res == CURLE_OK);
#else
    (void)endpoint; (void)data; (void)data_len;
    return false;  /* Network not supported */
#endif
}

/* ============================================================================
 * LOGGING
 * ============================================================================ */

void consumption_platform_log(int level, const char* message) {
    const char* level_str[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    const char* level_color[] = {"\033[31m", "\033[33m", "\033[36m", "\033[37m"};

    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(log_file, "[%s] %s: %s\n", timestamp, level_str[level], message);
        fclose(log_file);
    }

    /* Also print to console with colors */
    if (level >= 0 && level <= 3) {
        printf("%s[%s] %s\033[0m\n", level_color[level], level_str[level], message);
    }
}

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

void* consumption_platform_malloc(size_t size) {
    return malloc(size);
}

void consumption_platform_free(void* ptr) {
    free(ptr);
}

/* ============================================================================
 * THREADING
 * ============================================================================ */

void consumption_platform_enter_critical(void) {
    pthread_mutex_lock(&g_mutex);
}

void consumption_platform_exit_critical(void) {
    pthread_mutex_unlock(&g_mutex);
}

/* ============================================================================
 * PLATFORM INIT/DEINIT
 * ============================================================================ */

bool consumption_platform_init(void) {
    /* Initialize curl for HTTPS */
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return false;
    }

    g_curl = curl_easy_init();
    if (!g_curl) {
        curl_global_cleanup();
        return false;
    }

    /* Disable SSL verification for development (enable in production!) */
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    return true;
}

void consumption_platform_deinit(void) {
    if (g_curl) {
        curl_easy_cleanup(g_curl);
        g_curl = NULL;
    }
    curl_global_cleanup();
}
