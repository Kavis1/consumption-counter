/**
 * @file test_basic.c
 * @brief Basic unit tests for Consumption Counter Module
 */

#include "consumption.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Mock platform functions for testing */
uint32_t mock_timestamp = 1000000000; /* 2001-09-09 01:46:40 UTC */

uint32_t consumption_platform_get_timestamp(void) {
    return mock_timestamp++;
}

bool consumption_platform_storage_read(void* data, size_t size) {
    memset(data, 0, size);
    return true;
}

bool consumption_platform_storage_write(const void* data, size_t size) {
    (void)data; (void)size;
    return true;
}

bool consumption_platform_network_send(const char* endpoint, const char* data, size_t data_len) {
    (void)endpoint; (void)data; (void)data_len;
    printf("MOCK: Network send called\n");
    return true;
}

void consumption_platform_log(int level, const char* message) {
    (void)level;
    printf("LOG: %s\n", message);
}

void* consumption_platform_malloc(size_t size) {
    return malloc(size);
}

void consumption_platform_free(void* ptr) {
    free(ptr);
}

void consumption_platform_enter_critical(void) {}
void consumption_platform_exit_critical(void) {}

bool consumption_platform_init(void) { return true; }
void consumption_platform_deinit(void) {}

/* Test functions */
void test_initialization(void) {
    printf("Testing initialization...\n");

    /* Test default initialization */
    consumption_error_t result = consumption_init(NULL);
    assert(result == CONSUMPTION_SUCCESS);

    /* Test deinitialization */
    result = consumption_deinit();
    assert(result == CONSUMPTION_SUCCESS);

    /* Test custom configuration */
    consumption_config_t config = {
        .machine_id = 12345,
        .enable_external_api = false,
        .ring_buffer_size = 50,
        .aggregation_interval = 300,
    };

    result = consumption_init(&config);
    assert(result == CONSUMPTION_SUCCESS);

    consumption_config_t check_config;
    result = consumption_get_config(&check_config);
    assert(result == CONSUMPTION_SUCCESS);
    assert(check_config.machine_id == 12345);
    assert(check_config.ring_buffer_size == 50);

    consumption_deinit();

    printf("✓ Initialization tests passed\n");
}

void test_dispense_events(void) {
    printf("Testing dispense events...\n");

    consumption_config_t config = {
        .machine_id = 67890,
        .ring_buffer_size = 10,
    };

    consumption_error_t result = consumption_init(&config);
    assert(result == CONSUMPTION_SUCCESS);

    /* Test valid dispense events */
    for (uint8_t product_id = 1; product_id <= 5; product_id++) {
        result = consumption_on_dispense(67890, product_id);
        assert(result == CONSUMPTION_SUCCESS);
    }

    /* Test invalid machine ID */
    result = consumption_on_dispense(99999, 1);
    assert(result == CONSUMPTION_ERROR_INVALID_PARAMETER);

    /* Test invalid product ID */
    result = consumption_on_dispense(67890, 0);
    assert(result == CONSUMPTION_ERROR_INVALID_PARAMETER);

    /* Check statistics */
    uint32_t total_events, buffered_events, last_sync;
    result = consumption_get_stats(&total_events, &buffered_events, &last_sync);
    assert(result == CONSUMPTION_SUCCESS);
    assert(total_events == 5);
    assert(buffered_events == 5);

    consumption_deinit();

    printf("✓ Dispense event tests passed\n");
}

void test_ring_buffer_overflow(void) {
    printf("Testing ring buffer overflow...\n");

    consumption_config_t config = {
        .machine_id = 11111,
        .ring_buffer_size = 3, /* Very small buffer */
    };

    consumption_error_t result = consumption_init(&config);
    assert(result == CONSUMPTION_SUCCESS);

    /* Fill buffer */
    for (uint8_t i = 1; i <= 3; i++) {
        result = consumption_on_dispense(11111, i);
        assert(result == CONSUMPTION_SUCCESS);
    }

    /* Check buffer is full */
    uint32_t buffered_events;
    consumption_get_stats(NULL, &buffered_events, NULL);
    assert(buffered_events == 3);

    /* Add more events (should overwrite oldest) */
    for (uint8_t i = 4; i <= 6; i++) {
        result = consumption_on_dispense(11111, i);
        assert(result == CONSUMPTION_SUCCESS);
    }

    /* Buffer should still be at max size */
    consumption_get_stats(NULL, &buffered_events, NULL);
    assert(buffered_events == 3);

    /* Total events should reflect all calls */
    uint32_t total_events;
    consumption_get_stats(&total_events, NULL, NULL);
    assert(total_events == 6);

    consumption_deinit();

    printf("✓ Ring buffer overflow tests passed\n");
}

void test_error_handling(void) {
    printf("Testing error handling...\n");

    /* Test invalid config */
    consumption_config_t invalid_config = {
        .machine_id = 0, /* Invalid */
        .ring_buffer_size = 1000,
    };

    consumption_error_t result = consumption_init(&invalid_config);
    assert(result == CONSUMPTION_ERROR_INVALID_CONFIG);

    /* Test uninitialized calls */
    result = consumption_on_dispense(12345, 1);
    assert(result == CONSUMPTION_ERROR_INVALID_CONFIG);

    /* Test version and error strings */
    const char* version = consumption_get_version();
    assert(version != NULL);
    assert(strlen(version) > 0);

    const char* error_str = consumption_error_string(CONSUMPTION_ERROR_INVALID_CONFIG);
    assert(error_str != NULL);
    assert(strlen(error_str) > 0);

    printf("✓ Error handling tests passed\n");
}

void test_configuration_update(void) {
    printf("Testing configuration updates...\n");

    consumption_config_t config = {
        .machine_id = 22222,
        .enable_external_api = false,
        .ring_buffer_size = 100,
        .aggregation_interval = 3600,
    };

    consumption_error_t result = consumption_init(&config);
    assert(result == CONSUMPTION_SUCCESS);

    /* Test valid config update */
    consumption_config_t new_config = config;
    new_config.aggregation_interval = 1800; /* Change allowed parameter */

    result = consumption_update_config(&new_config);
    assert(result == CONSUMPTION_SUCCESS);

    /* Test invalid config update (ring buffer size change) */
    new_config.ring_buffer_size = 200; /* Not allowed at runtime */
    result = consumption_update_config(&new_config);
    assert(result == CONSUMPTION_ERROR_INVALID_PARAMETER);

    consumption_deinit();

    printf("✓ Configuration update tests passed\n");
}

int main(void) {
    printf("Consumption Counter Module - Basic Tests\n");
    printf("========================================\n\n");

    test_initialization();
    test_dispense_events();
    test_ring_buffer_overflow();
    test_error_handling();
    test_configuration_update();

    printf("\n✓ All basic tests passed!\n");
    return 0;
}
