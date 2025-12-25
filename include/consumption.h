/**
 * @file consumption.h
 * @brief Anonymous Consumption Counter Module
 *
 * Vendor-friendly module for vending machines to track beverage dispensing
 * events anonymously without affecting core functionality.
 *
 * Version: 1.0 (Pilot → Enterprise-ready)
 *
 * Features:
 * - Anonymous event tracking
 * - Local ring buffer storage
 * - Optional external API integration
 * - GDPR compliant (no personal data)
 * - Fail-safe operation
 */

#ifndef CONSUMPTION_H
#define CONSUMPTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Configuration structure for the consumption module
 */
typedef struct {
    uint32_t machine_id;              /**< Unique machine identifier */
    bool enable_external_api;         /**< Enable/disable external API (default: false) */
    uint32_t ring_buffer_size;        /**< Size of ring buffer for events (default: 1000) */
    uint32_t aggregation_interval;    /**< Aggregation interval in seconds (default: 3600) */
    char api_endpoint[256];           /**< External API endpoint URL */
    char api_key[128];                /**< API authentication key (optional) */
    uint32_t max_retry_attempts;      /**< Max retry attempts for API calls (default: 3) */
} consumption_config_t;

/* ============================================================================
 * EVENT STRUCTURES
 * ============================================================================ */

/**
 * @brief Individual consumption event
 */
typedef struct {
    uint32_t timestamp;     /**< Unix timestamp */
    uint32_t machine_id;    /**< Machine identifier */
    uint8_t product_id;     /**< Product identifier (1-255) */
} consumption_event_t;

/**
 * @brief Aggregated consumption data
 */
typedef struct {
    uint32_t machine_id;           /**< Machine identifier */
    uint32_t period_start;         /**< Start of aggregation period */
    uint32_t period_end;           /**< End of aggregation period */
    uint32_t total_events;         /**< Total number of events */
    uint32_t product_counts[256];  /**< Count per product ID */
} consumption_aggregate_t;

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */

typedef enum {
    CONSUMPTION_SUCCESS = 0,
    CONSUMPTION_ERROR_INVALID_CONFIG = 1,
    CONSUMPTION_ERROR_STORAGE_FULL = 2,
    CONSUMPTION_ERROR_NETWORK_UNAVAILABLE = 3,
    CONSUMPTION_ERROR_API_ERROR = 4,
    CONSUMPTION_ERROR_MEMORY_ERROR = 5,
    CONSUMPTION_ERROR_INVALID_PARAMETER = 6
} consumption_error_t;

/* ============================================================================
 * CORE API (MANDATORY)
 * ============================================================================ */

/**
 * @brief Initialize the consumption module
 *
 * Must be called once during system initialization.
 * This function is thread-safe and can be called multiple times.
 *
 * @param config Pointer to configuration structure
 * @return CONSUMPTION_SUCCESS on success, error code otherwise
 */
consumption_error_t consumption_init(const consumption_config_t* config);

/**
 * @brief Record a beverage dispensing event
 *
 * Core function that must be called after successful beverage dispensing.
 * This function must complete in ≤ 5ms and not block the main thread.
 *
 * @param machine_id Unique machine identifier
 * @param product_id Product identifier (1-255)
 * @return CONSUMPTION_SUCCESS on success, error code otherwise
 */
consumption_error_t consumption_on_dispense(uint32_t machine_id, uint8_t product_id);

/**
 * @brief Deinitialize the consumption module
 *
 * Should be called during system shutdown.
 * Ensures all pending data is flushed to persistent storage.
 *
 * @return CONSUMPTION_SUCCESS on success, error code otherwise
 */
consumption_error_t consumption_deinit(void);

/* ============================================================================
 * OPTIONAL LIFECYCLE EVENTS
 * ============================================================================ */

/**
 * @brief Called during system boot
 *
 * Optional: Implement if additional boot-time initialization is needed.
 */
void consumption_on_boot(void);

/**
 * @brief Called during system shutdown
 *
 * Optional: Implement if cleanup is needed beyond deinit().
 */
void consumption_on_shutdown(void);

/**
 * @brief Report an error condition
 *
 * Optional: Call when the vending machine encounters an error.
 * This helps with debugging and analytics.
 *
 * @param error_code Vendor-specific error code
 */
void consumption_on_error(uint16_t error_code);

/* ============================================================================
 * STATISTICS AND MONITORING
 * ============================================================================ */

/**
 * @brief Get current statistics
 *
 * @param total_events Pointer to store total event count
 * @param buffered_events Pointer to store events in buffer
 * @param last_sync Pointer to store last sync timestamp
 * @return CONSUMPTION_SUCCESS on success
 */
consumption_error_t consumption_get_stats(uint32_t* total_events,
                                        uint32_t* buffered_events,
                                        uint32_t* last_sync);

/**
 * @brief Force synchronization of buffered data
 *
 * Manually trigger data sync to external API (if enabled).
 * This is a blocking operation and should be used carefully.
 *
 * @return CONSUMPTION_SUCCESS on success, error code otherwise
 */
consumption_error_t consumption_force_sync(void);

/* ============================================================================
 * CONFIGURATION MANAGEMENT
 * ============================================================================ */

/**
 * @brief Update module configuration at runtime
 *
 * Allows dynamic reconfiguration without restart.
 * Not all parameters may be changeable at runtime.
 *
 * @param config Pointer to new configuration
 * @return CONSUMPTION_SUCCESS on success, error code otherwise
 */
consumption_error_t consumption_update_config(const consumption_config_t* config);

/**
 * @brief Get current configuration
 *
 * @param config Pointer to configuration structure to fill
 * @return CONSUMPTION_SUCCESS on success
 */
consumption_error_t consumption_get_config(consumption_config_t* config);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get version information
 *
 * @return Version string (e.g., "1.0.0")
 */
const char* consumption_get_version(void);

/**
 * @brief Get error description
 *
 * @param error Error code
 * @return Human-readable error description
 */
const char* consumption_error_string(consumption_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* CONSUMPTION_H */
