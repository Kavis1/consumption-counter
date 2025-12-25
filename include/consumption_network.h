/**
 * @file consumption_network.h
 * @brief Network client interfaces for Consumption Counter Module
 *
 * Optional network module providing HTTPS REST API and MQTT client functionality.
 * This module can be integrated into platform implementations.
 */

#ifndef CONSUMPTION_NETWORK_H
#define CONSUMPTION_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * COMMON TYPES
 * ============================================================================ */

/**
 * @brief Network transport types
 */
typedef enum {
    CONSUMPTION_NETWORK_NONE = 0,
    CONSUMPTION_NETWORK_HTTPS = 1,
    CONSUMPTION_NETWORK_MQTT = 2,
    CONSUMPTION_NETWORK_TCP = 3,
} consumption_network_type_t;

/**
 * @brief Network error codes
 */
typedef enum {
    CONSUMPTION_NETWORK_SUCCESS = 0,
    CONSUMPTION_NETWORK_ERROR_INIT = 1,
    CONSUMPTION_NETWORK_ERROR_CONNECT = 2,
    CONSUMPTION_NETWORK_ERROR_TIMEOUT = 3,
    CONSUMPTION_NETWORK_ERROR_AUTH = 4,
    CONSUMPTION_NETWORK_ERROR_SSL = 5,
    CONSUMPTION_NETWORK_ERROR_SEND = 6,
    CONSUMPTION_NETWORK_ERROR_RECEIVE = 7,
    CONSUMPTION_NETWORK_ERROR_UNKNOWN = 8,
} consumption_network_error_t;

/**
 * @brief Network configuration
 */
typedef struct {
    consumption_network_type_t type;
    char server[256];
    uint16_t port;
    char username[64];
    char password[128];
    char client_id[64];
    uint32_t timeout_ms;
    bool use_ssl;
    char ca_cert_path[256];
    char client_cert_path[256];
    char client_key_path[256];
} consumption_network_config_t;

/* ============================================================================
 * HTTPS CLIENT
 * ============================================================================ */

/**
 * @brief HTTPS client handle
 */
typedef struct consumption_https_client_t consumption_https_client_t;

/**
 * @brief Initialize HTTPS client
 *
 * @param config Network configuration
 * @return HTTPS client handle or NULL on error
 */
consumption_https_client_t* consumption_https_init(const consumption_network_config_t* config);

/**
 * @brief Send HTTPS POST request
 *
 * @param client HTTPS client handle
 * @param endpoint API endpoint path (appended to server URL)
 * @param data JSON payload
 * @param data_len Length of payload
 * @param response_code Pointer to store HTTP response code (can be NULL)
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_https_post(consumption_https_client_t* client,
                                                  const char* endpoint,
                                                  const char* data,
                                                  size_t data_len,
                                                  long* response_code);

/**
 * @brief Deinitialize HTTPS client
 *
 * @param client HTTPS client handle
 */
void consumption_https_deinit(consumption_https_client_t* client);

/* ============================================================================
 * MQTT CLIENT
 * ============================================================================ */

/**
 * @brief MQTT client handle
 */
typedef struct consumption_mqtt_client_t consumption_mqtt_client_t;

/**
 * @brief MQTT message callback
 *
 * @param topic MQTT topic
 * @param payload Message payload
 * @param payload_len Payload length
 * @param user_data User data passed to callback
 */
typedef void (*consumption_mqtt_message_callback_t)(const char* topic,
                                                  const char* payload,
                                                  size_t payload_len,
                                                  void* user_data);

/**
 * @brief Initialize MQTT client
 *
 * @param config Network configuration
 * @param message_callback Callback for incoming messages (can be NULL)
 * @param user_data User data for callback
 * @return MQTT client handle or NULL on error
 */
consumption_mqtt_client_t* consumption_mqtt_init(const consumption_network_config_t* config,
                                               consumption_mqtt_message_callback_t message_callback,
                                               void* user_data);

/**
 * @brief Connect to MQTT broker
 *
 * @param client MQTT client handle
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_mqtt_connect(consumption_mqtt_client_t* client);

/**
 * @brief Publish message to MQTT topic
 *
 * @param client MQTT client handle
 * @param topic MQTT topic
 * @param payload Message payload
 * @param payload_len Payload length
 * @param qos Quality of Service (0, 1, or 2)
 * @param retain Retain flag
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_mqtt_publish(consumption_mqtt_client_t* client,
                                                   const char* topic,
                                                   const char* payload,
                                                   size_t payload_len,
                                                   int qos,
                                                   bool retain);

/**
 * @brief Subscribe to MQTT topic
 *
 * @param client MQTT client handle
 * @param topic MQTT topic
 * @param qos Quality of Service (0, 1, or 2)
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_mqtt_subscribe(consumption_mqtt_client_t* client,
                                                     const char* topic,
                                                     int qos);

/**
 * @brief Process MQTT network loop (must be called periodically)
 *
 * @param client MQTT client handle
 * @param timeout_ms Timeout in milliseconds
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_mqtt_loop(consumption_mqtt_client_t* client,
                                                int timeout_ms);

/**
 * @brief Disconnect from MQTT broker
 *
 * @param client MQTT client handle
 * @return CONSUMPTION_NETWORK_SUCCESS on success
 */
consumption_network_error_t consumption_mqtt_disconnect(consumption_mqtt_client_t* client);

/**
 * @brief Deinitialize MQTT client
 *
 * @param client MQTT client handle
 */
void consumption_mqtt_deinit(consumption_mqtt_client_t* client);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get error description
 *
 * @param error Error code
 * @return Human-readable error description
 */
const char* consumption_network_error_string(consumption_network_error_t error);

/**
 * @brief Create default HTTPS configuration
 *
 * @param config Configuration to initialize
 * @param server Server hostname or IP
 * @param api_key API authentication key
 */
void consumption_network_https_config_default(consumption_network_config_t* config,
                                            const char* server,
                                            const char* api_key);

/**
 * @brief Create default MQTT configuration
 *
 * @param config Configuration to initialize
 * @param server Server hostname or IP
 * @param client_id MQTT client ID
 * @param username MQTT username (can be NULL)
 * @param password MQTT password (can be NULL)
 */
void consumption_network_mqtt_config_default(consumption_network_config_t* config,
                                           const char* server,
                                           const char* client_id,
                                           const char* username,
                                           const char* password);

/* ============================================================================
 * CONVENIENCE FUNCTIONS FOR CONSUMPTION MODULE
 * ============================================================================ */

/**
 * @brief Send consumption data via HTTPS
 *
 * Convenience function that handles JSON formatting and HTTPS posting.
 *
 * @param server Server URL
 * @param api_key API key for authentication
 * @param machine_id Machine identifier
 * @param period_start Start of aggregation period
 * @param period_end End of aggregation period
 * @param total_events Total number of events
 * @param product_counts Array of product counts (256 elements)
 * @return true on success
 */
bool consumption_network_send_https_data(const char* server,
                                       const char* api_key,
                                       uint32_t machine_id,
                                       uint32_t period_start,
                                       uint32_t period_end,
                                       uint32_t total_events,
                                       const uint32_t product_counts[256]);

/**
 * @brief Send consumption data via MQTT
 *
 * Convenience function that handles JSON formatting and MQTT publishing.
 *
 * @param server MQTT broker server
 * @param client_id MQTT client ID
 * @param username MQTT username (can be NULL)
 * @param password MQTT password (can be NULL)
 * @param topic_base Base topic for publishing
 * @param machine_id Machine identifier
 * @param period_start Start of aggregation period
 * @param period_end End of aggregation period
 * @param total_events Total number of events
 * @param product_counts Array of product counts (256 elements)
 * @return true on success
 */
bool consumption_network_send_mqtt_data(const char* server,
                                      const char* client_id,
                                      const char* username,
                                      const char* password,
                                      const char* topic_base,
                                      uint32_t machine_id,
                                      uint32_t period_start,
                                      uint32_t period_end,
                                      uint32_t total_events,
                                      const uint32_t product_counts[256]);

#ifdef __cplusplus
}
#endif

#endif /* CONSUMPTION_NETWORK_H */
