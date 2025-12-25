/**
 * @file vending_machine_integration.c
 * @brief Example integration of Consumption Counter Module in a vending machine
 *
 * This example demonstrates how to integrate the consumption module into
 * an existing vending machine firmware without affecting core functionality.
 */

#include "consumption.h"
#include "consumption_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ============================================================================
 * VENDING MACHINE SIMULATION
 * ============================================================================ */

/**
 * @brief Simulated vending machine state
 */
typedef struct {
    uint32_t machine_id;
    uint8_t product_count;
    char products[10][32];  /* Product names */
    uint8_t prices[10];     /* Product prices in cents */
    uint32_t cash_balance;  /* Current cash balance */
} vending_machine_t;

/**
 * @brief Initialize vending machine
 */
void vending_init(vending_machine_t* vm, uint32_t machine_id) {
    vm->machine_id = machine_id;
    vm->product_count = 5;
    vm->cash_balance = 0;

    strcpy(vm->products[1], "Coffee");
    strcpy(vm->products[2], "Tea");
    strcpy(vm->products[3], "Hot Chocolate");
    strcpy(vm->products[4], "Cappuccino");
    strcpy(vm->products[5], "Latte");

    vm->prices[1] = 150;  /* $1.50 */
    vm->prices[2] = 120;  /* $1.20 */
    vm->prices[3] = 180;  /* $1.80 */
    vm->prices[4] = 200;  /* $2.00 */
    vm->prices[5] = 220;  /* $2.20 */
}

/**
 * @brief Simulate payment processing
 */
bool process_payment(vending_machine_t* vm, uint8_t product_id, uint32_t payment_amount) {
    if (product_id == 0 || product_id > vm->product_count) {
        printf("Invalid product ID\n");
        return false;
    }

    if (payment_amount < vm->prices[product_id]) {
        printf("Insufficient payment\n");
        return false;
    }

    vm->cash_balance += payment_amount;
    return true;
}

/**
 * @brief Dispense beverage (core vending function)
 *
 * This is where the consumption tracking is integrated.
 */
bool dispense_beverage(vending_machine_t* vm, uint8_t product_id) {
    /* Simulate dispensing time */
    printf("Dispensing %s...\n", vm->products[product_id]);
    sleep(1); /* Simulate 1 second dispensing */

    /* CRITICAL: Call consumption tracking after successful dispensing */
    consumption_error_t result = consumption_on_dispense(vm->machine_id, product_id);
    if (result != CONSUMPTION_SUCCESS) {
        /* Log error but don't fail the dispensing */
        printf("Warning: Consumption tracking failed: %s\n",
               consumption_error_string(result));
    }

    printf("✓ %s dispensed successfully!\n", vm->products[product_id]);
    return true;
}

/**
 * @brief Complete beverage purchase
 */
bool purchase_beverage(vending_machine_t* vm, uint8_t product_id, uint32_t payment_amount) {
    /* Step 1: Process payment (existing logic) */
    if (!process_payment(vm, product_id, payment_amount)) {
        return false;
    }

    /* Step 2: Dispense beverage (with consumption tracking) */
    if (!dispense_beverage(vm, product_id)) {
        /* Handle dispensing failure */
        return false;
    }

    /* Step 3: Calculate and return change */
    uint32_t change = payment_amount - vm->prices[product_id];
    if (change > 0) {
        printf("Returning change: $%d.%02d\n", change / 100, change % 100);
        vm->cash_balance -= change;
    }

    return true;
}

/* ============================================================================
 * CONSUMPTION MODULE CONFIGURATION
 * ============================================================================ */

/**
 * @brief Configure consumption module for this vending machine
 */
void setup_consumption_module(vending_machine_t* vm) {
    consumption_config_t config = {
        .machine_id = vm->machine_id,
        .enable_external_api = true,  /* Enable for demo */
        .ring_buffer_size = 100,      /* Smaller buffer for demo */
        .aggregation_interval = 60,   /* Aggregate every minute for demo */
        .max_retry_attempts = 3,
    };

    strcpy(config.api_endpoint, "https://api.example.com/vending/consumption");
    strcpy(config.api_key, "demo-api-key-12345");

    consumption_error_t result = consumption_init(&config);
    if (result != CONSUMPTION_SUCCESS) {
        printf("Failed to initialize consumption module: %s\n",
               consumption_error_string(result));
        exit(1);
    }

    printf("✓ Consumption module initialized for machine %u\n", vm->machine_id);
}

/* ============================================================================
 * DEMO APPLICATION
 * ============================================================================ */

int main(int argc, char* argv[]) {
    printf("Vending Machine with Consumption Tracking Demo\n");
    printf("===============================================\n\n");

    /* Initialize platform layer */
    if (!consumption_platform_init()) {
        printf("Failed to initialize platform\n");
        return 1;
    }

    /* Create vending machine */
    vending_machine_t vm;
    vending_init(&vm, 12345); /* Machine ID: 12345 */

    /* Setup consumption tracking */
    setup_consumption_module(&vm);

    /* Optional: Register lifecycle events */
    consumption_on_boot();

    /* Demo: Simulate some purchases */
    printf("\nSimulating beverage purchases...\n");
    printf("================================\n");

    struct {
        uint8_t product_id;
        uint32_t payment;
    } purchases[] = {
        {1, 200}, /* Coffee with $2.00 */
        {3, 180}, /* Hot Chocolate with $1.80 */
        {2, 150}, /* Tea with $1.50 */
        {5, 220}, /* Latte with $2.20 */
        {1, 150}, /* Coffee with exact change */
        {4, 300}, /* Cappuccino with extra payment */
    };

    for (size_t i = 0; i < sizeof(purchases) / sizeof(purchases[0]); i++) {
        printf("\nPurchase %zu: Product %d ($%d.%02d paid)\n",
               i + 1, purchases[i].product_id,
               purchases[i].payment / 100, purchases[i].payment % 100);

        if (purchase_beverage(&vm, purchases[i].product_id, purchases[i].payment)) {
            printf("✓ Purchase completed\n");
        } else {
            printf("✗ Purchase failed\n");
        }

        /* Small delay between purchases */
        sleep(1);
    }

    /* Show consumption statistics */
    printf("\nConsumption Statistics:\n");
    printf("======================\n");

    uint32_t total_events, buffered_events, last_sync;
    consumption_error_t result = consumption_get_stats(&total_events, &buffered_events, &last_sync);
    if (result == CONSUMPTION_SUCCESS) {
        printf("Total events recorded: %u\n", total_events);
        printf("Events in buffer: %u\n", buffered_events);
        printf("Last sync timestamp: %u\n", last_sync);
    }

    /* Force synchronization to show API call */
    printf("\nForcing data synchronization...\n");
    result = consumption_force_sync();
    if (result == CONSUMPTION_SUCCESS) {
        printf("✓ Data synchronized\n");
    } else {
        printf("⚠ Sync failed: %s\n", consumption_error_string(result));
    }

    /* Simulate error reporting */
    printf("\nSimulating vending machine error...\n");
    consumption_on_error(42); /* Error code 42 */

    /* Shutdown sequence */
    printf("\nShutting down...\n");
    consumption_on_shutdown();
    consumption_deinit();
    consumption_platform_deinit();

    printf("✓ Demo completed successfully!\n");
    return 0;
}
