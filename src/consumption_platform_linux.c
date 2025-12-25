/**
 * @file consumption_platform_linux.c
 * @brief Linux platform implementation for Consumption Counter Module
 *
 * Implementation for Linux systems using system calls and standard libraries.
 * Supports file-based storage, system time, and network via curl or sockets.
 */

#include "consumption_platform.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <syslog.h>
#include <pthread.h>
#include <curl/curl.h>
#include <errno.h>

/* Configuration */
#define STORAGE_FILE "/var/lib/consumption-data.bin"
#define LOG_IDENT "consumption-module"
#define MAX_STORAGE_SIZE 4096

/* Global variables */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static CURL* g_curl = NULL;
static int g_log_to_syslog = 0;
static FILE* g_log_file = NULL;

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
    if (size > MAX_STORAGE_SIZE) {
        return false;
    }

    int fd = open(STORAGE_FILE, O_RDONLY);
    if (fd < 0) {
        /* File doesn't exist yet, return zeros */
        memset(data, 0, size);
        return true;
    }

    ssize_t bytes_read = read(fd, data, size);
    close(fd);

    if (bytes_read < 0) {
        return false;
    }

    /* If file is smaller than requested, zero-fill the rest */
    if ((size_t)bytes_read < size) {
        memset((char*)data + bytes_read, 0, size - bytes_read);
    }

    return true;
}

bool consumption_platform_storage_write(const void* data, size_t size) {
    if (size > MAX_STORAGE_SIZE) {
        return false;
    }

    /* Create directory if it doesn't exist */
    char dir_path[256];
    strcpy(dir_path, STORAGE_FILE);
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        mkdir(dir_path, 0755);  /* Create directory with proper permissions */
    }

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
    if (!g_curl) {
        return false;
    }

    CURLcode res;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: Consumption-Module/1.0");

    curl_easy_setopt(g_curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(g_curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(g_curl, CURLOPT_POSTFIELDSIZE, data_len);
    curl_easy_setopt(g_curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(g_curl, CURLOPT_TIMEOUT, 30L);  /* 30 second timeout */
    curl_easy_setopt(g_curl, CURLOPT_CONNECTTIMEOUT, 10L);  /* 10 second connect timeout */

    /* Disable SSL verification for development (enable in production!) */
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(g_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(g_curl);

    curl_slist_free_all(headers);

    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(g_curl, CURLINFO_RESPONSE_CODE, &response_code);

        /* Consider 2xx responses as success */
        return (response_code >= 200 && response_code < 300);
    }

    return false;
}

/* ============================================================================
 * LOGGING
 * ============================================================================ */

void consumption_platform_log(int level, const char* message) {
    /* Convert our log levels to syslog levels */
    int syslog_level;
    const char* level_str;

    switch (level) {
        case 0: syslog_level = LOG_ERR;    level_str = "ERROR"; break;
        case 1: syslog_level = LOG_WARNING; level_str = "WARN";  break;
        case 2: syslog_level = LOG_INFO;   level_str = "INFO";  break;
        case 3: syslog_level = LOG_DEBUG;  level_str = "DEBUG"; break;
        default: syslog_level = LOG_INFO;  level_str = "INFO";  break;
    }

    /* Log to syslog if enabled */
    if (g_log_to_syslog) {
        syslog(syslog_level, "[%s] %s", level_str, message);
    }

    /* Log to console/file */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    char log_msg[512];
    int len = snprintf(log_msg, sizeof(log_msg), "[%s] [%s] %s\n",
                      timestamp, level_str, message);

    if (len > 0) {
        /* Print to stderr */
        fputs(log_msg, stderr);

        /* Write to log file if configured */
        if (g_log_file) {
            fputs(log_msg, g_log_file);
            fflush(g_log_file);
        }
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

    /* Check if we should use syslog */
    g_log_to_syslog = (getenv("CONSUMPTION_USE_SYSLOG") != NULL);

    if (g_log_to_syslog) {
        openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_USER);
    }

    /* Check for log file */
    const char* log_file_path = getenv("CONSUMPTION_LOG_FILE");
    if (log_file_path) {
        g_log_file = fopen(log_file_path, "a");
        if (g_log_file) {
            setvbuf(g_log_file, NULL, _IOLBF, 0);  /* Line buffered */
        }
    }

    return true;
}

void consumption_platform_deinit(void) {
    if (g_curl) {
        curl_easy_cleanup(g_curl);
        g_curl = NULL;
    }
    curl_global_cleanup();

    if (g_log_to_syslog) {
        closelog();
    }

    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

/* ============================================================================
 * LINUX SPECIFIC HELPERS
 * ============================================================================ */

/**
 * @brief Set custom storage file path
 * @param file_path Path to storage file
 * @return true on success
 */
bool consumption_linux_set_storage_file(const char* file_path) {
    /* This function allows overriding the default storage file */
    /* Implementation would modify STORAGE_FILE define */
    (void)file_path;  /* Placeholder */
    return true;
}

/**
 * @brief Enable/disable syslog logging
 * @param enable true to enable syslog
 */
void consumption_linux_set_syslog(bool enable) {
    g_log_to_syslog = enable;
    if (enable && !g_log_to_syslog) {
        openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_USER);
    } else if (!enable && g_log_to_syslog) {
        closelog();
    }
    g_log_to_syslog = enable;
}

/**
 * @brief Set custom log file
 * @param file_path Path to log file (NULL to disable file logging)
 * @return true on success
 */
bool consumption_linux_set_log_file(const char* file_path) {
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }

    if (file_path) {
        g_log_file = fopen(file_path, "a");
        if (g_log_file) {
            setvbuf(g_log_file, NULL, _IOLBF, 0);  /* Line buffered */
            return true;
        }
        return false;
    }

    return true;
}

/**
 * @brief Get system uptime in milliseconds
 * @return Uptime in milliseconds
 */
uint32_t consumption_linux_get_uptime_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint32_t)((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
    }
    return 0;
}

/**
 * @brief Check if running as root
 * @return true if running as root
 */
bool consumption_linux_is_root(void) {
    return (geteuid() == 0);
}

/**
 * @brief Ensure proper file permissions for storage
 * @return true on success
 */
bool consumption_linux_ensure_permissions(void) {
    /* Ensure storage directory exists with proper permissions */
    char dir_path[256];
    strcpy(dir_path, STORAGE_FILE);
    char* last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (mkdir(dir_path, 0755) == 0 || errno == EEXIST) {
            /* Set proper permissions on storage file */
            chmod(STORAGE_FILE, 0644);
            return true;
        }
    }
    return false;
}
