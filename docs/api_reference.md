# Consumption Counter Module - API Reference

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue?logo=github)](https://github.com/Kavis1/consumption-counter)
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)](https://github.com/Kavis1/consumption-counter/releases)

> **Note:** This API reference is part of the Consumption Counter Module hosted on [GitHub](https://github.com/Kavis1/consumption-counter). For the latest updates and source code, visit the repository.

Complete API documentation for the Consumption Counter Module.

## Table of Contents

- [Core API](#core-api)
  - [Initialization](#initialization)
  - [Event Registration](#event-registration)
  - [Configuration](#configuration)
  - [Statistics](#statistics)
  - [Lifecycle](#lifecycle)
- [Network API](#network-api)
  - [HTTPS Client](#https-client)
  - [MQTT Client](#mqtt-client)
  - [Convenience Functions](#convenience-functions)
- [Platform API](#platform-api)
- [Data Types](#data-types)
- [Error Codes](#error-codes)
- [Constants](#constants)

## Core API

### Initialization

#### `consumption_init()`

```c
consumption_error_t consumption_init(const consumption_config_t* config);
```

Initializes the consumption module with the provided configuration.

**Parameters:**
- `config`: Pointer to configuration structure. Pass `NULL` for default configuration.

**Returns:**
- `CONSUMPTION_SUCCESS` on success
- `CONSUMPTION_ERROR_INVALID_CONFIG` if configuration is invalid

**Thread Safety:** Not thread-safe (call once during system initialization)

**Example:**
```c
consumption_config_t config = {
    .machine_id = 12345,
    .enable_external_api = false,
    .ring_buffer_size = 1000,
    .aggregation_interval = 3600,
};

consumption_error_t result = consumption_init(&config);
if (result != CONSUMPTION_SUCCESS) {
    // Handle error
}
```

---

#### `consumption_deinit()`

```c
consumption_error_t consumption_deinit(void);
```

Deinitializes the consumption module and flushes persistent storage.

**Returns:**
- `CONSUMPTION_SUCCESS` on success

**Thread Safety:** Not thread-safe (call once during system shutdown)

---

### Event Registration

#### `consumption_on_dispense()`

```c
consumption_error_t consumption_on_dispense(uint32_t machine_id, uint8_t product_id);
```

**Core function** - Registers a beverage dispensing event.

⚠️ **Critical**: This function must be called AFTER successful beverage dispensing only.

**Parameters:**
- `machine_id`: Unique machine identifier (must match configuration)
- `product_id`: Product identifier (1-255)

**Returns:**
- `CONSUMPTION_SUCCESS` on success
- `CONSUMPTION_ERROR_INVALID_PARAMETER` if parameters are invalid

**Performance:**
- Execution time: < 5ms
- Thread-safe: Yes
- Blocking: No

**Example:**
```c
// In vending machine dispense function
void dispense_beverage(uint8_t product_id) {
    // ... dispensing logic ...

    // Register consumption event
    consumption_on_dispense(machine_id, product_id);
}
```

---

### Configuration

#### `consumption_update_config()`

```c
consumption_error_t consumption_update_config(const consumption_config_t* config);
```

Updates module configuration at runtime.

**Note:** Not all configuration parameters can be changed at runtime.

**Parameters:**
- `config`: Pointer to new configuration structure

**Returns:**
- `CONSUMPTION_SUCCESS` on success
- `CONSUMPTION_ERROR_INVALID_CONFIG` if configuration is invalid

---

#### `consumption_get_config()`

```c
consumption_error_t consumption_get_config(consumption_config_t* config);
```

Retrieves current module configuration.

**Parameters:**
- `config`: Pointer to configuration structure to fill

**Returns:**
- `CONSUMPTION_SUCCESS` on success

---

### Statistics

#### `consumption_get_stats()`

```c
consumption_error_t consumption_get_stats(
    uint32_t* total_events,
    uint32_t* buffered_events,
    uint32_t* last_sync
);
```

Retrieves current module statistics.

**Parameters:**
- `total_events`: Pointer to store total events count (can be NULL)
- `buffered_events`: Pointer to store buffered events count (can be NULL)
- `last_sync`: Pointer to store last sync timestamp (can be NULL)

**Returns:**
- `CONSUMPTION_SUCCESS` on success

**Example:**
```c
uint32_t total, buffered, last_sync;
if (consumption_get_stats(&total, &buffered, &last_sync) == CONSUMPTION_SUCCESS) {
    printf("Total events: %u, Buffered: %u, Last sync: %u\n",
           total, buffered, last_sync);
}
```

---

#### `consumption_force_sync()`

```c
consumption_error_t consumption_force_sync(void);
```

Manually triggers data synchronization to external API.

⚠️ **Warning**: This is a blocking operation and should be used carefully.

**Returns:**
- `CONSUMPTION_SUCCESS` on success

---

### Lifecycle

#### `consumption_on_boot()`

```c
void consumption_on_boot(void);
```

Optional: Registers system boot event.

**When to call:** After `consumption_init()` during system startup.

---

#### `consumption_on_shutdown()`

```c
void consumption_on_shutdown(void);
```

Optional: Registers system shutdown event.

**When to call:** Before `consumption_deinit()` during system shutdown.

---

#### `consumption_on_error()`

```c
void consumption_on_error(uint16_t error_code);
```

Optional: Reports vending machine errors for debugging and analytics.

**Parameters:**
- `error_code`: Vendor-specific error code

**Example:**
```c
// In error handling code
consumption_on_error(42);  // Error code 42
```

---

#### `consumption_get_version()`

```c
const char* consumption_get_version(void);
```

Returns module version string.

**Returns:** Version string (e.g., "1.0.0")

---

## Network API

### HTTPS Client

#### `consumption_https_init()`

```c
consumption_https_client_t* consumption_https_init(const consumption_network_config_t* config);
```

Initializes HTTPS client with configuration.

**Parameters:**
- `config`: Network configuration (type must be `CONSUMPTION_NETWORK_HTTPS`)

**Returns:** HTTPS client handle or NULL on error

---

#### `consumption_https_post()`

```c
consumption_network_error_t consumption_https_post(
    consumption_https_client_t* client,
    const char* endpoint,
    const char* data,
    size_t data_len,
    long* response_code
);
```

Sends HTTPS POST request.

**Parameters:**
- `client`: HTTPS client handle
- `endpoint`: API endpoint path (appended to server URL)
- `data`: Request payload
- `data_len`: Payload length
- `response_code`: Pointer to store HTTP response code (can be NULL)

**Returns:** Network error code

---

#### `consumption_https_deinit()`

```c
void consumption_https_deinit(consumption_https_client_t* client);
```

Deinitializes HTTPS client.

**Parameters:**
- `client`: HTTPS client handle

---

### MQTT Client

#### `consumption_mqtt_init()`

```c
consumption_mqtt_client_t* consumption_mqtt_init(
    const consumption_network_config_t* config,
    consumption_mqtt_message_callback_t message_callback,
    void* user_data
);
```

Initializes MQTT client.

**Parameters:**
- `config`: Network configuration (type must be `CONSUMPTION_NETWORK_MQTT`)
- `message_callback`: Callback for incoming messages (can be NULL)
- `user_data`: User data for callback

**Returns:** MQTT client handle or NULL on error

---

#### `consumption_mqtt_connect()`

```c
consumption_network_error_t consumption_mqtt_connect(consumption_mqtt_client_t* client);
```

Connects to MQTT broker.

**Parameters:**
- `client`: MQTT client handle

**Returns:** Network error code

---

#### `consumption_mqtt_publish()`

```c
consumption_network_error_t consumption_mqtt_publish(
    consumption_mqtt_client_t* client,
    const char* topic,
    const char* payload,
    size_t payload_len,
    int qos,
    bool retain
);
```

Publishes message to MQTT topic.

**Parameters:**
- `client`: MQTT client handle
- `topic`: MQTT topic
- `payload`: Message payload
- `payload_len`: Payload length
- `qos`: Quality of Service (0, 1, or 2)
- `retain`: Retain flag

**Returns:** Network error code

---

#### `consumption_mqtt_subscribe()`

```c
consumption_network_error_t consumption_mqtt_subscribe(
    consumption_mqtt_client_t* client,
    const char* topic,
    int qos
);
```

Subscribes to MQTT topic.

**Parameters:**
- `client`: MQTT client handle
- `topic`: MQTT topic
- `qos`: Quality of Service (0, 1, or 2)

**Returns:** Network error code

---

#### `consumption_mqtt_loop()`

```c
consumption_network_error_t consumption_mqtt_loop(
    consumption_mqtt_client_t* client,
    int timeout_ms
);
```

Processes MQTT network loop (call periodically).

**Parameters:**
- `client`: MQTT client handle
- `timeout_ms`: Timeout in milliseconds

**Returns:** Network error code

---

#### `consumption_mqtt_disconnect()`

```c
consumption_network_error_t consumption_mqtt_disconnect(consumption_mqtt_client_t* client);
```

Disconnects from MQTT broker.

**Parameters:**
- `client`: MQTT client handle

**Returns:** Network error code

---

#### `consumption_mqtt_deinit()`

```c
void consumption_mqtt_deinit(consumption_mqtt_client_t* client);
```

Deinitializes MQTT client.

**Parameters:**
- `client`: MQTT client handle

---

### Convenience Functions

#### `consumption_network_send_https_data()`

```c
bool consumption_network_send_https_data(
    const char* server,
    const char* api_key,
    uint32_t machine_id,
    uint32_t period_start,
    uint32_t period_end,
    uint32_t total_events,
    const uint32_t product_counts[256]
);
```

Convenience function to send consumption data via HTTPS.

**Returns:** true on success

---

#### `consumption_network_send_mqtt_data()`

```c
bool consumption_network_send_mqtt_data(
    const char* server,
    const char* client_id,
    const char* username,
    const char* password,
    const char* topic_base,
    uint32_t machine_id,
    uint32_t period_start,
    uint32_t period_end,
    uint32_t total_events,
    const uint32_t product_counts[256]
);
```

Convenience function to send consumption data via MQTT.

**Returns:** true on success

---

## Platform API

### Time Functions

#### `consumption_platform_get_timestamp()`

```c
uint32_t consumption_platform_get_timestamp(void);
```

Returns current Unix timestamp in seconds.

**Returns:** Current timestamp

---

### Storage Functions

#### `consumption_platform_storage_read()`

```c
bool consumption_platform_storage_read(void* data, size_t size);
```

Reads data from persistent storage.

**Parameters:**
- `data`: Buffer to read into
- `size`: Number of bytes to read

**Returns:** true on success

---

#### `consumption_platform_storage_write()`

```c
bool consumption_platform_storage_write(const void* data, size_t size);
```

Writes data to persistent storage.

**Parameters:**
- `data`: Buffer to write from
- `size`: Number of bytes to write

**Returns:** true on success

---

### Network Functions

#### `consumption_platform_network_send()`

```c
bool consumption_platform_network_send(
    const char* endpoint,
    const char* data,
    size_t data_len
);
```

Sends data to external API endpoint.

**Parameters:**
- `endpoint`: API endpoint URL
- `data`: JSON payload
- `data_len`: Payload length

**Returns:** true on success

---

### Logging Functions

#### `consumption_platform_log()`

```c
void consumption_platform_log(int level, const char* message);
```

Logs a message.

**Parameters:**
- `level`: Log level (0=error, 1=warning, 2=info, 3=debug)
- `message`: Log message

---

### Memory Functions

#### `consumption_platform_malloc()`

```c
void* consumption_platform_malloc(size_t size);
```

Allocates memory.

**Parameters:**
- `size`: Number of bytes to allocate

**Returns:** Pointer to allocated memory or NULL

---

#### `consumption_platform_free()`

```c
void consumption_platform_free(void* ptr);
```

Frees memory.

**Parameters:**
- `ptr`: Pointer to free (can be NULL)

---

### Threading Functions

#### `consumption_platform_enter_critical()`

```c
void consumption_platform_enter_critical(void);
```

Enters critical section.

---

#### `consumption_platform_exit_critical()`

```c
void consumption_platform_exit_critical(void);
```

Exits critical section.

---

### Lifecycle Functions

#### `consumption_platform_init()`

```c
bool consumption_platform_init(void);
```

Initializes platform-specific resources.

**Returns:** true on success

---

#### `consumption_platform_deinit()`

```c
void consumption_platform_deinit(void);
```

Deinitializes platform-specific resources.

---

## Data Types

### `consumption_config_t`

```c
typedef struct {
    uint32_t machine_id;              // Unique machine ID
    bool enable_external_api;         // Enable external API sync
    uint32_t ring_buffer_size;        // Ring buffer size (1-10000)
    uint32_t aggregation_interval;    // Sync interval in seconds
    char api_endpoint[256];           // API server URL
    char api_key[128];                // Authentication key
    uint32_t max_retry_attempts;      // Max retry attempts
} consumption_config_t;
```

Configuration structure for the consumption module.

---

### `consumption_event_t`

```c
typedef struct {
    uint32_t timestamp;     // Unix timestamp (seconds)
    uint32_t machine_id;    // Machine identifier
    uint8_t product_id;     // Product ID (1-255)
} consumption_event_t;
```

Individual consumption event structure.

---

### `consumption_aggregate_t`

```c
typedef struct {
    uint32_t machine_id;           // Machine identifier
    uint32_t period_start;         // Aggregation period start
    uint32_t period_end;           // Aggregation period end
    uint32_t total_events;         // Total events in period
    uint32_t product_counts[256];  // Count per product ID
} consumption_aggregate_t;
```

Aggregated consumption data structure.

---

### `consumption_network_config_t`

```c
typedef struct {
    consumption_network_type_t type;  // Network type
    char server[256];                  // Server hostname/IP
    uint16_t port;                     // Server port
    char username[64];                 // Username (optional)
    char password[128];                // Password/API key
    char client_id[64];                // Client ID
    uint32_t timeout_ms;               // Timeout in milliseconds
    bool use_ssl;                      // Enable SSL/TLS
    char ca_cert_path[256];            // CA certificate path
    char client_cert_path[256];        // Client certificate path
    char client_key_path[256];         // Client key path
} consumption_network_config_t;
```

Network configuration structure.

---

## Error Codes

### Core Module Errors

| Error Code | Value | Description |
|------------|-------|-------------|
| `CONSUMPTION_SUCCESS` | 0 | Operation successful |
| `CONSUMPTION_ERROR_INVALID_CONFIG` | 1 | Invalid configuration |
| `CONSUMPTION_ERROR_STORAGE_FULL` | 2 | Storage full |
| `CONSUMPTION_ERROR_NETWORK_UNAVAILABLE` | 3 | Network unavailable |
| `CONSUMPTION_ERROR_API_ERROR` | 4 | API error |
| `CONSUMPTION_ERROR_MEMORY_ERROR` | 5 | Memory allocation error |
| `CONSUMPTION_ERROR_INVALID_PARAMETER` | 6 | Invalid parameter |

### Network Errors

| Error Code | Value | Description |
|------------|-------|-------------|
| `CONSUMPTION_NETWORK_SUCCESS` | 0 | Operation successful |
| `CONSUMPTION_NETWORK_ERROR_INIT` | 1 | Initialization error |
| `CONSUMPTION_NETWORK_ERROR_CONNECT` | 2 | Connection error |
| `CONSUMPTION_NETWORK_ERROR_TIMEOUT` | 3 | Timeout error |
| `CONSUMPTION_NETWORK_ERROR_AUTH` | 4 | Authentication error |
| `CONSUMPTION_NETWORK_ERROR_SSL` | 5 | SSL/TLS error |
| `CONSUMPTION_NETWORK_ERROR_SEND` | 6 | Send error |
| `CONSUMPTION_NETWORK_ERROR_RECEIVE` | 7 | Receive error |
| `CONSUMPTION_NETWORK_ERROR_UNKNOWN` | 8 | Unknown error |

---

## Constants

### Configuration Limits

| Constant | Value | Description |
|----------|-------|-------------|
| `MAX_MACHINE_ID` | N/A | Maximum machine ID value |
| `MIN_RING_BUFFER_SIZE` | 1 | Minimum ring buffer size |
| `MAX_RING_BUFFER_SIZE` | 10000 | Maximum ring buffer size |
| `MIN_PRODUCT_ID` | 1 | Minimum product ID |
| `MAX_PRODUCT_ID` | 255 | Maximum product ID |

### Network Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `DEFAULT_HTTPS_PORT` | 443 | Default HTTPS port |
| `DEFAULT_MQTT_PORT` | 1883 | Default MQTT port |
| `DEFAULT_TIMEOUT_MS` | 30000 | Default timeout (30 seconds) |
| `MAX_ENDPOINT_LENGTH` | 256 | Maximum endpoint URL length |
| `MAX_API_KEY_LENGTH` | 128 | Maximum API key length |

### Log Levels

| Level | Value | Description |
|-------|-------|-------------|
| `LOG_ERROR` | 0 | Error messages |
| `LOG_WARNING` | 1 | Warning messages |
| `LOG_INFO` | 2 | Information messages |
| `LOG_DEBUG` | 3 | Debug messages |

---

## Helper Functions

### `consumption_error_string()`

```c
const char* consumption_error_string(consumption_error_t error);
```

Converts error code to human-readable string.

**Parameters:**
- `error`: Error code

**Returns:** Error description string

---

### `consumption_network_error_string()`

```c
const char* consumption_network_error_string(consumption_network_error_t error);
```

Converts network error code to human-readable string.

**Parameters:**
- `error`: Network error code

**Returns:** Error description string

---

### Configuration Helpers

#### `consumption_network_https_config_default()`

```c
void consumption_network_https_config_default(
    consumption_network_config_t* config,
    const char* server,
    const char* api_key
);
```

Creates default HTTPS configuration.

---

#### `consumption_network_mqtt_config_default()`

```c
void consumption_network_mqtt_config_default(
    consumption_network_config_t* config,
    const char* server,
    const char* client_id,
    const char* username,
    const char* password
);
```

Creates default MQTT configuration.

---

## Platform-Specific Notes

### STM32 Platform

- Uses STM32 HAL for Flash operations
- Requires RTC for timestamp
- UART logging supported
- HTTPS/MQTT require external libraries

### NXP Platform

- Uses MCUXpresso SDK
- SNVS HP RTC for timestamp
- LPUART for logging
- FlexSPI NOR Flash support

### Linux Platform

- Uses POSIX file operations
- system time for timestamp
- syslog/file logging
- Native curl/mosquitto integration

### POSIX Platform

- Generic Unix compatibility
- Standard C library functions
- File-based storage
- Console logging

---

For more information, see the [Integration Guide](integration_guide.md) and [README](../README.md).

**[🔙 Back to Main Documentation](../README.md)** | **[📖 Integration Guide](integration_guide.md)** | **[🤝 Contributing](../CONTRIBUTING.md)** | **[🧪 Live Demo](../examples/)**
