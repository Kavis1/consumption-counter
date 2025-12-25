/**
 * @file consumption_platform.h
 * @brief Platform abstraction layer for Consumption Counter Module
 *
 * This file defines the platform-specific functions that must be implemented
 * to port the consumption module to different hardware platforms.
 *
 * Each platform integration must provide implementations for these functions.
 */

#ifndef CONSUMPTION_PLATFORM_H
#define CONSUMPTION_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TIME FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get current Unix timestamp
 *
 * @return Current timestamp in seconds since Unix epoch (1970-01-01 00:00:00 UTC)
 *
 * Implementation notes:
 * - For embedded systems without RTC, use uptime counter + initial offset
 * - Must be monotonic and reasonably accurate
 * - Should not roll over during device lifetime
 */
uint32_t consumption_platform_get_timestamp(void);

/* ============================================================================
 * PERSISTENT STORAGE
 * ============================================================================ */

/**
 * @brief Read data from persistent storage
 *
 * @param data Buffer to read data into
 * @param size Number of bytes to read
 * @return true on success, false on failure
 *
 * Implementation notes:
 * - Use Flash, EEPROM, or filesystem as appropriate
 * - Should be atomic operation (use wear-leveling if needed)
 * - Size will typically be sizeof(consumption_state_t) + buffer size
 * - Must survive power loss
 */
bool consumption_platform_storage_read(void* data, size_t size);

/**
 * @brief Write data to persistent storage
 *
 * @param data Buffer containing data to write
 * @param size Number of bytes to write
 * @return true on success, false on failure
 *
 * Implementation notes:
 * - Same storage as read function
 * - Should be atomic (consider journaling for complex data)
 * - Must survive power loss
 * - May be slow, so minimize calls
 */
bool consumption_platform_storage_write(const void* data, size_t size);

/* ============================================================================
 * NETWORKING (OPTIONAL)
 * ============================================================================ */

/**
 * @brief Send data to external API endpoint
 *
 * This function is only called when external API is enabled.
 * Implementation is optional - can return false if networking unavailable.
 *
 * @param endpoint API endpoint URL (e.g., "https://api.example.com/consumption")
 * @param data JSON payload to send
 * @param data_len Length of payload in bytes
 * @return true on success, false on failure
 *
 * Implementation notes:
 * - Support HTTPS for production use
 * - MQTT support can be added as alternative transport
 * - Should be non-blocking or have reasonable timeout
 * - Authentication via API key in headers if needed
 * - Return false if network unavailable (module will retry later)
 */
bool consumption_platform_network_send(const char* endpoint,
                                     const char* data,
                                     size_t data_len);

/* ============================================================================
 * LOGGING (OPTIONAL)
 * ============================================================================ */

/**
 * @brief Log a message
 *
 * Optional function for debugging and monitoring.
 * Can be implemented as no-op if logging not available.
 *
 * @param level Log level: 0=error, 1=warning, 2=info, 3=debug
 * @param message Null-terminated log message
 *
 * Implementation notes:
 * - Can write to serial console, file, or ignore
 * - Should not block for long periods
 * - Use platform's logging facility if available
 */
void consumption_platform_log(int level, const char* message);

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

/**
 * @brief Allocate memory
 *
 * Platform-specific memory allocation.
 * Use this instead of standard malloc() for better control.
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 *
 * Implementation notes:
 * - For RTOS: use pvPortMalloc or equivalent
 * - For bare-metal: implement simple heap
 * - Should be thread-safe if needed
 */
void* consumption_platform_malloc(size_t size);

/**
 * @brief Free memory
 *
 * @param ptr Pointer to memory to free (can be NULL)
 */
void consumption_platform_free(void* ptr);

/* ============================================================================
 * THREADING/PRIMITIVES (OPTIONAL)
 * ============================================================================ */

/**
 * @brief Enter critical section
 *
 * Optional: Implement if multi-threading is possible.
 * Prevents concurrent access to module data.
 */
void consumption_platform_enter_critical(void);

/**
 * @brief Exit critical section
 */
void consumption_platform_exit_critical(void);

/* ============================================================================
 * PLATFORM SPECIFIC INITIALIZATION
 * ============================================================================ */

/**
 * @brief Platform-specific initialization
 *
 * Called during consumption_init().
 * Initialize hardware resources needed by the platform functions.
 *
 * @return true on success
 */
bool consumption_platform_init(void);

/**
 * @brief Platform-specific deinitialization
 *
 * Called during consumption_deinit().
 * Clean up hardware resources.
 */
void consumption_platform_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* CONSUMPTION_PLATFORM_H */
