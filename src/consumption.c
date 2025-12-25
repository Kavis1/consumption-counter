/**
 * @file consumption.c
 * @brief Anonymous Consumption Counter Module Implementation
 *
 * C99 compliant implementation for embedded systems.
 * Designed for MCU/RTOS environments with limited resources.
 */

#include "consumption.h"
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * PLATFORM ABSTRACTIONS
 * ============================================================================ */

/*
 * These functions need to be implemented by the platform integration layer.
 * They provide hardware/storage abstraction.
 */

/**
 * @brief Get current Unix timestamp
 * @return Current timestamp in seconds since epoch
 */
extern uint32_t consumption_platform_get_timestamp(void);

/**
 * @brief Persistent storage read
 * @param data Buffer to read into
 * @param size Size of data to read
 * @return true on success
 */
extern bool consumption_platform_storage_read(void* data, size_t size);

/**
 * @brief Persistent storage write
 * @param data Buffer to write from
 * @param size Size of data to write
 * @return true on success
 */
extern bool consumption_platform_storage_write(const void* data, size_t size);

/**
 * @brief Network send function (optional)
 * @param endpoint API endpoint URL
 * @param data JSON payload
 * @param data_len Length of payload
 * @return true on success
 */
extern bool consumption_platform_network_send(const char* endpoint,
                                            const char* data,
                                            size_t data_len);

/**
 * @brief Logging function (optional)
 * @param level Log level (0=error, 1=warn, 2=info, 3=debug)
 * @param message Log message
 */
extern void consumption_platform_log(int level, const char* message);

/* ============================================================================
 * INTERNAL STRUCTURES
 * ============================================================================ */

/**
 * @brief Internal module state
 */
typedef struct {
    bool initialized;
    consumption_config_t config;
    uint32_t total_events;
    uint32_t buffer_head;
    uint32_t buffer_tail;
    uint32_t buffer_count;
    consumption_event_t* event_buffer;
    uint32_t last_aggregation;
    uint32_t last_sync;
    bool sync_in_progress;
} consumption_state_t;

/* ============================================================================
 * GLOBAL STATE
 * ============================================================================ */

static consumption_state_t g_state = {0};

/* ============================================================================
 * INTERNAL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Validate configuration
 */
static bool validate_config(const consumption_config_t* config) {
    if (!config) return false;
    if (config->machine_id == 0) return false;
    if (config->ring_buffer_size == 0) return false;
    if (config->ring_buffer_size > 10000) return false; /* Reasonable limit */
    return true;
}

/**
 * @brief Initialize default configuration
 */
static void init_default_config(consumption_config_t* config) {
    memset(config, 0, sizeof(consumption_config_t));
    config->enable_external_api = false;
    config->ring_buffer_size = 1000;
    config->aggregation_interval = 3600; /* 1 hour */
    config->max_retry_attempts = 3;
    strncpy(config->api_endpoint, "https://api.example.com/consumption", sizeof(config->api_endpoint) - 1);
}

/**
 * @brief Save state to persistent storage
 */
static bool save_state(void) {
    return consumption_platform_storage_write(&g_state, sizeof(g_state));
}

/**
 * @brief Load state from persistent storage
 */
static bool load_state(void) {
    return consumption_platform_storage_read(&g_state, sizeof(g_state));
}

/**
 * @brief Add event to ring buffer
 */
static consumption_error_t add_event_to_buffer(const consumption_event_t* event) {
    if (g_state.buffer_count >= g_state.config.ring_buffer_size) {
        /* Buffer full, overwrite oldest */
        g_state.buffer_tail = (g_state.buffer_tail + 1) % g_state.config.ring_buffer_size;
        g_state.buffer_count--;
    }

    g_state.event_buffer[g_state.buffer_head] = *event;
    g_state.buffer_head = (g_state.buffer_head + 1) % g_state.config.ring_buffer_size;
    g_state.buffer_count++;

    return CONSUMPTION_SUCCESS;
}

/**
 * @brief Aggregate events into summary data
 */
static void aggregate_events(consumption_aggregate_t* aggregate,
                           uint32_t start_time, uint32_t end_time) {
    memset(aggregate, 0, sizeof(consumption_aggregate_t));
    aggregate->machine_id = g_state.config.machine_id;
    aggregate->period_start = start_time;
    aggregate->period_end = end_time;

    uint32_t index = g_state.buffer_tail;
    for (uint32_t i = 0; i < g_state.buffer_count; i++) {
        const consumption_event_t* event = &g_state.event_buffer[index];
        if (event->timestamp >= start_time && event->timestamp <= end_time) {
            aggregate->total_events++;
            if (event->product_id < 256) {
                aggregate->product_counts[event->product_id]++;
            }
        }
        index = (index + 1) % g_state.config.ring_buffer_size;
    }
}

/**
 * @brief Send aggregated data to external API
 */
static consumption_error_t sync_to_api(void) {
    if (!g_state.config.enable_external_api) {
        return CONSUMPTION_SUCCESS;
    }

    if (g_state.sync_in_progress) {
        return CONSUMPTION_ERROR_API_ERROR; /* Already in progress */
    }

    g_state.sync_in_progress = true;

    uint32_t now = consumption_platform_get_timestamp();
    uint32_t period_start = g_state.last_aggregation;
    uint32_t period_end = now;

    if (period_end - period_start < g_state.config.aggregation_interval) {
        g_state.sync_in_progress = false;
        return CONSUMPTION_SUCCESS; /* Not enough time passed */
    }

    consumption_aggregate_t aggregate;
    aggregate_events(&aggregate, period_start, period_end);

    if (aggregate.total_events == 0) {
        g_state.sync_in_progress = false;
        return CONSUMPTION_SUCCESS; /* Nothing to send */
    }

    /* Create JSON payload */
    char json_buffer[1024];
    int len = snprintf(json_buffer, sizeof(json_buffer),
        "{\"machine_id\":%u,\"period_start\":%u,\"period_end\":%u,\"total_events\":%u",
        aggregate.machine_id, aggregate.period_start, aggregate.period_end, aggregate.total_events);

    /* Add product counts */
    bool first = true;
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",\"products\":{");
    for (int i = 1; i < 256; i++) {
        if (aggregate.product_counts[i] > 0) {
            if (!first) len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",");
            len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "\"%d\":%u", i, aggregate.product_counts[i]);
            first = false;
        }
    }
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "}}");

    bool success = consumption_platform_network_send(
        g_state.config.api_endpoint,
        json_buffer,
        (size_t)len
    );

    g_state.sync_in_progress = false;

    if (success) {
        g_state.last_sync = now;
        g_state.last_aggregation = period_end;
        save_state(); /* Persist sync state */
        consumption_platform_log(2, "Consumption data synced successfully");
        return CONSUMPTION_SUCCESS;
    } else {
        consumption_platform_log(0, "Failed to sync consumption data");
        return CONSUMPTION_ERROR_API_ERROR;
    }
}

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

consumption_error_t consumption_init(const consumption_config_t* config) {
    if (g_state.initialized) {
        return CONSUMPTION_SUCCESS; /* Already initialized */
    }

    consumption_config_t default_config;
    init_default_config(&default_config);

    if (config) {
        if (!validate_config(config)) {
            return CONSUMPTION_ERROR_INVALID_CONFIG;
        }
        g_state.config = *config;
    } else {
        g_state.config = default_config;
    }

    /* Try to load previous state */
    if (!load_state()) {
        /* First run or corrupted storage */
        memset(&g_state, 0, sizeof(g_state));
        g_state.config = config ? *config : default_config;
        g_state.last_aggregation = consumption_platform_get_timestamp();
    }

    /* Allocate ring buffer */
    g_state.event_buffer = (consumption_event_t*)malloc(
        g_state.config.ring_buffer_size * sizeof(consumption_event_t));
    if (!g_state.event_buffer) {
        return CONSUMPTION_ERROR_MEMORY_ERROR;
    }

    g_state.initialized = true;
    consumption_platform_log(2, "Consumption module initialized");

    return CONSUMPTION_SUCCESS;
}

consumption_error_t consumption_on_dispense(uint32_t machine_id, uint8_t product_id) {
    if (!g_state.initialized) {
        return CONSUMPTION_ERROR_INVALID_CONFIG;
    }

    if (machine_id != g_state.config.machine_id) {
        return CONSUMPTION_ERROR_INVALID_PARAMETER;
    }

    if (product_id == 0) {
        return CONSUMPTION_ERROR_INVALID_PARAMETER;
    }

    consumption_event_t event = {
        .timestamp = consumption_platform_get_timestamp(),
        .machine_id = machine_id,
        .product_id = product_id
    };

    consumption_error_t result = add_event_to_buffer(&event);
    if (result != CONSUMPTION_SUCCESS) {
        return result;
    }

    g_state.total_events++;

    /* Try to sync if enabled and interval passed */
    if (g_state.config.enable_external_api) {
        uint32_t now = consumption_platform_get_timestamp();
        if (now - g_state.last_sync >= g_state.config.aggregation_interval) {
            /* Non-blocking sync attempt */
            sync_to_api();
        }
    }

    return CONSUMPTION_SUCCESS;
}

consumption_error_t consumption_deinit(void) {
    if (!g_state.initialized) {
        return CONSUMPTION_SUCCESS;
    }

    /* Force final sync if enabled */
    if (g_state.config.enable_external_api) {
        sync_to_api();
    }

    /* Save final state */
    save_state();

    /* Free resources */
    free(g_state.event_buffer);
    g_state.event_buffer = NULL;

    g_state.initialized = false;
    consumption_platform_log(2, "Consumption module deinitialized");

    return CONSUMPTION_SUCCESS;
}

void consumption_on_boot(void) {
    /* Optional boot-time initialization */
    consumption_platform_log(2, "Consumption module boot event");
}

void consumption_on_shutdown(void) {
    /* Optional shutdown handling */
    consumption_platform_log(2, "Consumption module shutdown event");
}

void consumption_on_error(uint16_t error_code) {
    /* Log error for debugging */
    char msg[64];
    snprintf(msg, sizeof(msg), "Consumption module error: %u", error_code);
    consumption_platform_log(0, msg);
}

consumption_error_t consumption_get_stats(uint32_t* total_events,
                                        uint32_t* buffered_events,
                                        uint32_t* last_sync) {
    if (!g_state.initialized) {
        return CONSUMPTION_ERROR_INVALID_CONFIG;
    }

    if (total_events) *total_events = g_state.total_events;
    if (buffered_events) *buffered_events = g_state.buffer_count;
    if (last_sync) *last_sync = g_state.last_sync;

    return CONSUMPTION_SUCCESS;
}

consumption_error_t consumption_force_sync(void) {
    if (!g_state.initialized) {
        return CONSUMPTION_ERROR_INVALID_CONFIG;
    }

    return sync_to_api();
}

consumption_error_t consumption_update_config(const consumption_config_t* config) {
    if (!config || !validate_config(config)) {
        return CONSUMPTION_ERROR_INVALID_CONFIG;
    }

    /* Some parameters can't be changed at runtime */
    if (config->ring_buffer_size != g_state.config.ring_buffer_size) {
        return CONSUMPTION_ERROR_INVALID_PARAMETER;
    }

    g_state.config = *config;
    save_state();

    return CONSUMPTION_SUCCESS;
}

consumption_error_t consumption_get_config(consumption_config_t* config) {
    if (!config) {
        return CONSUMPTION_ERROR_INVALID_PARAMETER;
    }

    *config = g_state.config;
    return CONSUMPTION_SUCCESS;
}

const char* consumption_get_version(void) {
    return "1.0.0";
}

const char* consumption_error_string(consumption_error_t error) {
    switch (error) {
        case CONSUMPTION_SUCCESS:
            return "Success";
        case CONSUMPTION_ERROR_INVALID_CONFIG:
            return "Invalid configuration";
        case CONSUMPTION_ERROR_STORAGE_FULL:
            return "Storage full";
        case CONSUMPTION_ERROR_NETWORK_UNAVAILABLE:
            return "Network unavailable";
        case CONSUMPTION_ERROR_API_ERROR:
            return "API error";
        case CONSUMPTION_ERROR_MEMORY_ERROR:
            return "Memory allocation error";
        case CONSUMPTION_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        default:
            return "Unknown error";
    }
}
