/**
 * @file consumption_network.c
 * @brief Network client implementation for Consumption Counter Module
 *
 * Provides HTTPS REST API and MQTT client functionality using libcurl and mosquitto.
 */

#include "consumption_network.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Check for required libraries */
#ifdef USE_CURL
#include <curl/curl.h>
#endif

#ifdef USE_MOSQUITTO
#include <mosquitto.h>
#endif

/* ============================================================================
 * ERROR STRINGS
 * ============================================================================ */

const char* consumption_network_error_string(consumption_network_error_t error) {
    switch (error) {
        case CONSUMPTION_NETWORK_SUCCESS:
            return "Success";
        case CONSUMPTION_NETWORK_ERROR_INIT:
            return "Initialization error";
        case CONSUMPTION_NETWORK_ERROR_CONNECT:
            return "Connection error";
        case CONSUMPTION_NETWORK_ERROR_TIMEOUT:
            return "Timeout error";
        case CONSUMPTION_NETWORK_ERROR_AUTH:
            return "Authentication error";
        case CONSUMPTION_NETWORK_ERROR_SSL:
            return "SSL/TLS error";
        case CONSUMPTION_NETWORK_ERROR_SEND:
            return "Send error";
        case CONSUMPTION_NETWORK_ERROR_RECEIVE:
            return "Receive error";
        case CONSUMPTION_NETWORK_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

/* ============================================================================
 * CONFIGURATION HELPERS
 * ============================================================================ */

void consumption_network_https_config_default(consumption_network_config_t* config,
                                            const char* server,
                                            const char* api_key) {
    memset(config, 0, sizeof(consumption_network_config_t));
    config->type = CONSUMPTION_NETWORK_HTTPS;
    strncpy(config->server, server, sizeof(config->server) - 1);
    config->port = 443;
    config->timeout_ms = 30000;  /* 30 seconds */
    config->use_ssl = true;
    if (api_key) {
        strncpy(config->password, api_key, sizeof(config->password) - 1);
    }
}

void consumption_network_mqtt_config_default(consumption_network_config_t* config,
                                           const char* server,
                                           const char* client_id,
                                           const char* username,
                                           const char* password) {
    memset(config, 0, sizeof(consumption_network_config_t));
    config->type = CONSUMPTION_NETWORK_MQTT;
    strncpy(config->server, server, sizeof(config->server) - 1);
    config->port = 1883;
    config->timeout_ms = 10000;  /* 10 seconds */
    config->use_ssl = false;
    if (client_id) {
        strncpy(config->client_id, client_id, sizeof(config->client_id) - 1);
    }
    if (username) {
        strncpy(config->username, username, sizeof(config->username) - 1);
    }
    if (password) {
        strncpy(config->password, password, sizeof(config->password) - 1);
    }
}

/* ============================================================================
 * HTTPS CLIENT IMPLEMENTATION
 * ============================================================================ */

#ifdef USE_CURL

struct consumption_https_client_t {
    CURL* curl;
    consumption_network_config_t config;
};

consumption_https_client_t* consumption_https_init(const consumption_network_config_t* config) {
    if (!config || config->type != CONSUMPTION_NETWORK_HTTPS) {
        return NULL;
    }

    consumption_https_client_t* client = (consumption_https_client_t*)malloc(sizeof(consumption_https_client_t));
    if (!client) {
        return NULL;
    }

    memcpy(&client->config, config, sizeof(consumption_network_config_t));

    client->curl = curl_easy_init();
    if (!client->curl) {
        free(client);
        return NULL;
    }

    /* Configure SSL if enabled */
    if (config->use_ssl) {
        curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(client->curl, CURLOPT_SSL_VERIFYHOST, 2L);

        if (config->ca_cert_path[0]) {
            curl_easy_setopt(client->curl, CURLOPT_CAINFO, config->ca_cert_path);
        }
        if (config->client_cert_path[0]) {
            curl_easy_setopt(client->curl, CURLOPT_SSLCERT, config->client_cert_path);
        }
        if (config->client_key_path[0]) {
            curl_easy_setopt(client->curl, CURLOPT_SSLKEY, config->client_key_path);
        }
    }

    /* Set timeout */
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT_MS, config->timeout_ms);
    curl_easy_setopt(client->curl, CURLOPT_CONNECTTIMEOUT_MS, config->timeout_ms / 2);

    /* Set user agent */
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "Consumption-Module/1.0");

    return client;
}

consumption_network_error_t consumption_https_post(consumption_https_client_t* client,
                                                  const char* endpoint,
                                                  const char* data,
                                                  size_t data_len,
                                                  long* response_code) {
    if (!client || !client->curl || !endpoint || !data) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    /* Build full URL */
    char url[512];
    int url_len = snprintf(url, sizeof(url), "%s%s",
                          client->config.server, endpoint);

    if (url_len >= sizeof(url)) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    /* Set URL */
    curl_easy_setopt(client->curl, CURLOPT_URL, url);

    /* Set POST data */
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDSIZE, data_len);

    /* Set headers */
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (client->config.password[0]) {
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
                client->config.password);
        headers = curl_slist_append(headers, auth_header);
    }
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, headers);

    /* Perform request */
    CURLcode res = curl_easy_perform(client->curl);

    /* Get response code */
    if (response_code) {
        curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, response_code);
    }

    /* Clean up headers */
    curl_slist_free_all(headers);

    /* Convert curl error to our error type */
    if (res != CURLE_OK) {
        switch (res) {
            case CURLE_COULDNT_CONNECT:
            case CURLE_COULDNT_RESOLVE_HOST:
                return CONSUMPTION_NETWORK_ERROR_CONNECT;
            case CURLE_OPERATION_TIMEDOUT:
                return CONSUMPTION_NETWORK_ERROR_TIMEOUT;
            case CURLE_SSL_CONNECT_ERROR:
            case CURLE_SSL_CERTPROBLEM:
                return CONSUMPTION_NETWORK_ERROR_SSL;
            default:
                return CONSUMPTION_NETWORK_ERROR_SEND;
        }
    }

    return CONSUMPTION_NETWORK_SUCCESS;
}

void consumption_https_deinit(consumption_https_client_t* client) {
    if (client) {
        if (client->curl) {
            curl_easy_cleanup(client->curl);
        }
        free(client);
    }
}

#else /* USE_CURL not defined */

consumption_https_client_t* consumption_https_init(const consumption_network_config_t* config) {
    (void)config;
    return NULL;  /* HTTPS not supported */
}

consumption_network_error_t consumption_https_post(consumption_https_client_t* client,
                                                  const char* endpoint,
                                                  const char* data,
                                                  size_t data_len,
                                                  long* response_code) {
    (void)client; (void)endpoint; (void)data; (void)data_len; (void)response_code;
    return CONSUMPTION_NETWORK_ERROR_INIT;  /* HTTPS not supported */
}

void consumption_https_deinit(consumption_https_client_t* client) {
    (void)client;
}

#endif /* USE_CURL */

/* ============================================================================
 * MQTT CLIENT IMPLEMENTATION
 * ============================================================================ */

#ifdef USE_MOSQUITTO

struct consumption_mqtt_client_t {
    struct mosquitto* mosq;
    consumption_network_config_t config;
    consumption_mqtt_message_callback_t message_callback;
    void* user_data;
    bool connected;
};

static void mqtt_message_callback(struct mosquitto* mosq, void* obj,
                                const struct mosquitto_message* message) {
    consumption_mqtt_client_t* client = (consumption_mqtt_client_t*)obj;

    if (client->message_callback && message->payload) {
        client->message_callback(message->topic,
                               (const char*)message->payload,
                               message->payloadlen,
                               client->user_data);
    }
}

static void mqtt_connect_callback(struct mosquitto* mosq, void* obj, int rc) {
    consumption_mqtt_client_t* client = (consumption_mqtt_client_t*)obj;
    (void)mosq;

    if (rc == 0) {
        client->connected = true;
    } else {
        client->connected = false;
    }
}

static void mqtt_disconnect_callback(struct mosquitto* mosq, void* obj, int rc) {
    consumption_mqtt_client_t* client = (consumption_mqtt_client_t*)obj;
    (void)mosq; (void)rc;

    client->connected = false;
}

consumption_mqtt_client_t* consumption_mqtt_init(const consumption_network_config_t* config,
                                               consumption_mqtt_message_callback_t message_callback,
                                               void* user_data) {
    if (!config || config->type != CONSUMPTION_NETWORK_MQTT) {
        return NULL;
    }

    /* Initialize mosquitto library */
    mosquitto_lib_init();

    consumption_mqtt_client_t* client = (consumption_mqtt_client_t*)malloc(sizeof(consumption_mqtt_client_t));
    if (!client) {
        return NULL;
    }

    memcpy(&client->config, config, sizeof(consumption_network_config_t));
    client->message_callback = message_callback;
    client->user_data = user_data;
    client->connected = false;

    /* Create mosquitto client */
    client->mosq = mosquitto_new(config->client_id[0] ? config->client_id : NULL,
                               true, client);
    if (!client->mosq) {
        free(client);
        return NULL;
    }

    /* Set callbacks */
    mosquitto_connect_callback_set(client->mosq, mqtt_connect_callback);
    mosquitto_disconnect_callback_set(client->mosq, mqtt_disconnect_callback);
    mosquitto_message_callback_set(client->mosq, mqtt_message_callback);

    /* Set authentication if provided */
    if (config->username[0] && config->password[0]) {
        mosquitto_username_pw_set(client->mosq, config->username, config->password);
    }

    /* Set TLS if enabled */
    if (config->use_ssl) {
        int tls_result = mosquitto_tls_set(client->mosq,
                                          config->ca_cert_path[0] ? config->ca_cert_path : NULL,
                                          NULL,
                                          config->client_cert_path[0] ? config->client_cert_path : NULL,
                                          config->client_key_path[0] ? config->client_key_path : NULL,
                                          NULL);
        if (tls_result != MOSQ_ERR_SUCCESS) {
            mosquitto_destroy(client->mosq);
            free(client);
            return NULL;
        }

        mosquitto_tls_insecure_set(client->mosq, false);
    }

    return client;
}

consumption_network_error_t consumption_mqtt_connect(consumption_mqtt_client_t* client) {
    if (!client || !client->mosq) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    int rc = mosquitto_connect(client->mosq,
                             client->config.server,
                             client->config.port,
                             client->config.timeout_ms / 1000);

    if (rc != MOSQ_ERR_SUCCESS) {
        if (rc == MOSQ_ERR_ERRNO) {
            return CONSUMPTION_NETWORK_ERROR_CONNECT;
        }
        return CONSUMPTION_NETWORK_ERROR_UNKNOWN;
    }

    return CONSUMPTION_NETWORK_SUCCESS;
}

consumption_network_error_t consumption_mqtt_publish(consumption_mqtt_client_t* client,
                                                   const char* topic,
                                                   const char* payload,
                                                   size_t payload_len,
                                                   int qos,
                                                   bool retain) {
    if (!client || !client->mosq || !topic) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    int rc = mosquitto_publish(client->mosq, NULL, topic,
                             (int)payload_len, payload, qos, retain);

    if (rc != MOSQ_ERR_SUCCESS) {
        switch (rc) {
            case MOSQ_ERR_NO_CONN:
                return CONSUMPTION_NETWORK_ERROR_CONNECT;
            case MOSQ_ERR_ERRNO:
                return CONSUMPTION_NETWORK_ERROR_SEND;
            default:
                return CONSUMPTION_NETWORK_ERROR_UNKNOWN;
        }
    }

    return CONSUMPTION_NETWORK_SUCCESS;
}

consumption_network_error_t consumption_mqtt_subscribe(consumption_mqtt_client_t* client,
                                                     const char* topic,
                                                     int qos) {
    if (!client || !client->mosq || !topic) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    int rc = mosquitto_subscribe(client->mosq, NULL, topic, qos);

    if (rc != MOSQ_ERR_SUCCESS) {
        switch (rc) {
            case MOSQ_ERR_NO_CONN:
                return CONSUMPTION_NETWORK_ERROR_CONNECT;
            case MOSQ_ERR_ERRNO:
                return CONSUMPTION_NETWORK_ERROR_SEND;
            default:
                return CONSUMPTION_NETWORK_ERROR_UNKNOWN;
        }
    }

    return CONSUMPTION_NETWORK_SUCCESS;
}

consumption_network_error_t consumption_mqtt_loop(consumption_mqtt_client_t* client,
                                                int timeout_ms) {
    if (!client || !client->mosq) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    int rc = mosquitto_loop(client->mosq, timeout_ms, 1);

    if (rc != MOSQ_ERR_SUCCESS) {
        switch (rc) {
            case MOSQ_ERR_NO_CONN:
                return CONSUMPTION_NETWORK_ERROR_CONNECT;
            case MOSQ_ERR_ERRNO:
                return CONSUMPTION_NETWORK_ERROR_RECEIVE;
            default:
                return CONSUMPTION_NETWORK_ERROR_UNKNOWN;
        }
    }

    return CONSUMPTION_NETWORK_SUCCESS;
}

consumption_network_error_t consumption_mqtt_disconnect(consumption_mqtt_client_t* client) {
    if (!client || !client->mosq) {
        return CONSUMPTION_NETWORK_ERROR_INIT;
    }

    int rc = mosquitto_disconnect(client->mosq);

    if (rc != MOSQ_ERR_SUCCESS) {
        return CONSUMPTION_NETWORK_ERROR_UNKNOWN;
    }

    client->connected = false;
    return CONSUMPTION_NETWORK_SUCCESS;
}

void consumption_mqtt_deinit(consumption_mqtt_client_t* client) {
    if (client) {
        if (client->mosq) {
            mosquitto_disconnect(client->mosq);
            mosquitto_destroy(client->mosq);
        }
        free(client);
    }

    mosquitto_lib_cleanup();
}

#else /* USE_MOSQUITTO not defined */

consumption_mqtt_client_t* consumption_mqtt_init(const consumption_network_config_t* config,
                                               consumption_mqtt_message_callback_t message_callback,
                                               void* user_data) {
    (void)config; (void)message_callback; (void)user_data;
    return NULL;  /* MQTT not supported */
}

consumption_network_error_t consumption_mqtt_connect(consumption_mqtt_client_t* client) {
    (void)client;
    return CONSUMPTION_NETWORK_ERROR_INIT;
}

consumption_network_error_t consumption_mqtt_publish(consumption_mqtt_client_t* client,
                                                   const char* topic,
                                                   const char* payload,
                                                   size_t payload_len,
                                                   int qos,
                                                   bool retain) {
    (void)client; (void)topic; (void)payload; (void)payload_len; (void)qos; (void)retain;
    return CONSUMPTION_NETWORK_ERROR_INIT;
}

consumption_network_error_t consumption_mqtt_subscribe(consumption_mqtt_client_t* client,
                                                     const char* topic,
                                                     int qos) {
    (void)client; (void)topic; (void)qos;
    return CONSUMPTION_NETWORK_ERROR_INIT;
}

consumption_network_error_t consumption_mqtt_loop(consumption_mqtt_client_t* client,
                                                int timeout_ms) {
    (void)client; (void)timeout_ms;
    return CONSUMPTION_NETWORK_ERROR_INIT;
}

consumption_network_error_t consumption_mqtt_disconnect(consumption_mqtt_client_t* client) {
    (void)client;
    return CONSUMPTION_NETWORK_ERROR_INIT;
}

void consumption_mqtt_deinit(consumption_mqtt_client_t* client) {
    (void)client;
}

#endif /* USE_MOSQUITTO */

/* ============================================================================
 * CONVENIENCE FUNCTIONS
 * ============================================================================ */

bool consumption_network_send_https_data(const char* server,
                                       const char* api_key,
                                       uint32_t machine_id,
                                       uint32_t period_start,
                                       uint32_t period_end,
                                       uint32_t total_events,
                                       const uint32_t product_counts[256]) {
#ifdef USE_CURL
    consumption_network_config_t config;
    consumption_network_https_config_default(&config, server, api_key);

    consumption_https_client_t* client = consumption_https_init(&config);
    if (!client) {
        return false;
    }

    /* Create JSON payload */
    char json_buffer[2048];
    int len = snprintf(json_buffer, sizeof(json_buffer),
        "{\"machine_id\":%u,\"period_start\":%u,\"period_end\":%u,\"total_events\":%u",
        machine_id, period_start, period_end, total_events);

    /* Add product counts */
    bool first = true;
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",\"products\":{");
    for (int i = 1; i < 256 && len < sizeof(json_buffer) - 50; i++) {
        if (product_counts[i] > 0) {
            if (!first) len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",");
            len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "\"%d\":%u", i, product_counts[i]);
            first = false;
        }
    }
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "}}");

    long response_code;
    consumption_network_error_t result = consumption_https_post(client, "/api/consumption",
                                                               json_buffer, (size_t)len,
                                                               &response_code);

    consumption_https_deinit(client);

    return (result == CONSUMPTION_NETWORK_SUCCESS && response_code >= 200 && response_code < 300);
#else
    (void)server; (void)api_key; (void)machine_id; (void)period_start; (void)period_end;
    (void)total_events; (void)product_counts;
    return false;  /* HTTPS not supported */
#endif
}

bool consumption_network_send_mqtt_data(const char* server,
                                      const char* client_id,
                                      const char* username,
                                      const char* password,
                                      const char* topic_base,
                                      uint32_t machine_id,
                                      uint32_t period_start,
                                      uint32_t period_end,
                                      uint32_t total_events,
                                      const uint32_t product_counts[256]) {
#ifdef USE_MOSQUITTO
    consumption_network_config_t config;
    consumption_network_mqtt_config_default(&config, server, client_id, username, password);

    consumption_mqtt_client_t* client = consumption_mqtt_init(&config, NULL, NULL);
    if (!client) {
        return false;
    }

    if (consumption_mqtt_connect(client) != CONSUMPTION_NETWORK_SUCCESS) {
        consumption_mqtt_deinit(client);
        return false;
    }

    /* Create JSON payload */
    char json_buffer[2048];
    int len = snprintf(json_buffer, sizeof(json_buffer),
        "{\"machine_id\":%u,\"period_start\":%u,\"period_end\":%u,\"total_events\":%u",
        machine_id, period_start, period_end, total_events);

    /* Add product counts */
    bool first = true;
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",\"products\":{");
    for (int i = 1; i < 256 && len < sizeof(json_buffer) - 50; i++) {
        if (product_counts[i] > 0) {
            if (!first) len += snprintf(json_buffer + len, sizeof(json_buffer) - len, ",");
            len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "\"%d\":%u", i, product_counts[i]);
            first = false;
        }
    }
    len += snprintf(json_buffer + len, sizeof(json_buffer) - len, "}}");

    /* Create topic */
    char topic[256];
    snprintf(topic, sizeof(topic), "%s/consumption/%u", topic_base, machine_id);

    consumption_network_error_t result = consumption_mqtt_publish(client, topic,
                                                                json_buffer, (size_t)len,
                                                                1, false);

    consumption_mqtt_disconnect(client);
    consumption_mqtt_deinit(client);

    return (result == CONSUMPTION_NETWORK_SUCCESS);
#else
    (void)server; (void)client_id; (void)username; (void)password; (void)topic_base;
    (void)machine_id; (void)period_start; (void)period_end; (void)total_events; (void)product_counts;
    return false;  /* MQTT not supported */
#endif
}
