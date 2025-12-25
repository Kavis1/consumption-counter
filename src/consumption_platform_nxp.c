/**
 * @file consumption_platform_nxp.c
 * @brief NXP platform implementation for Consumption Counter Module
 *
 * Implementation for NXP microcontrollers using MCUXpresso SDK.
 * Supports Flash storage, RTC timestamp, and optional network via LWIP/FreeRTOS+TCP.
 */

#include "consumption_platform.h"
#include <string.h>
#include <stdlib.h>

/* NXP SDK includes - uncomment as needed */
/*
#include "fsl_flash.h"         // Flash operations
#include "fsl_snvs_hp.h"       // Secure Non-Volatile Storage
#include "fsl_snvs_lp.h"       // Low Power SNVS
#include "fsl_rtc.h"           // RTC
#include "fsl_lpuart.h"        // UART for logging
#include "fsl_common.h"        // Common utilities
*/

/* Configuration */
#define STORAGE_START_ADDRESS 0x60000000  /* FlexSPI NOR Flash or other non-volatile memory */
#define MAX_STORAGE_SIZE 4096             /* 4KB storage size */
#define LOG_BUFFER_SIZE 128

/* Global variables */
static char log_buffer[LOG_BUFFER_SIZE];

/* Flash handle for NXP SDK */
static flash_config_t s_flashDriver;

/* ============================================================================
 * TIME FUNCTIONS
 * ============================================================================ */

uint32_t consumption_platform_get_timestamp(void) {
    /* Using NXP SNVS or RTC */
    snvs_hp_rtc_datetime_t rtcDateTime;

    /* Get current time from SNVS HP RTC */
    SNVS_HP_RTC_GetDatetime(SNVS_HP_RTC, &rtcDateTime);

    /* Convert to Unix timestamp */
    struct tm time_struct = {
        .tm_sec = rtcDateTime.second,
        .tm_min = rtcDateTime.minute,
        .tm_hour = rtcDateTime.hour,
        .tm_mday = rtcDateTime.day,
        .tm_mon = rtcDateTime.month - 1,
        .tm_year = rtcDateTime.year - 1900,
        .tm_isdst = 0
    };

    return (uint32_t)mktime(&time_struct);
}

/* ============================================================================
 * PERSISTENT STORAGE
 * ============================================================================ */

bool consumption_platform_storage_read(void* data, size_t size) {
    if (size > MAX_STORAGE_SIZE) {
        return false;
    }

    /* Read from Flash or other non-volatile memory */
    status_t status = FLASH_Read(&s_flashDriver, STORAGE_START_ADDRESS,
                               (uint8_t*)data, size);

    return (status == kStatus_Success);
}

bool consumption_platform_storage_write(const void* data, size_t size) {
    if (size > MAX_STORAGE_SIZE) {
        return false;
    }

    status_t status;

    /* Erase flash sector first */
    status = FLASH_Erase(&s_flashDriver, STORAGE_START_ADDRESS,
                        size, kFLASH_ApiEraseKey);
    if (status != kStatus_Success) {
        return false;
    }

    /* Write data */
    status = FLASH_Program(&s_flashDriver, STORAGE_START_ADDRESS,
                          (uint32_t*)data, size);

    return (status == kStatus_Success);
}

/* ============================================================================
 * NETWORKING (OPTIONAL)
 * ============================================================================ */

bool consumption_platform_network_send(const char* endpoint,
                                     const char* data,
                                     size_t data_len) {
    /* Implementation depends on network stack */
    /* Example using LWIP or FreeRTOS+TCP */

#ifdef USE_LWIP
    /* LWIP implementation for NXP */
    /* This is a placeholder - actual implementation depends on your setup */

    // Create TCP socket
    // Connect to server
    // Send HTTP POST
    // Close connection

    return false;  /* Placeholder - not implemented */

#elif defined(USE_FREERTOS_TCP)
    /* FreeRTOS+TCP implementation */

    return false;  /* Placeholder - not implemented */

#else
    /* No network stack available */
    return false;
#endif
}

/* ============================================================================
 * LOGGING
 * ============================================================================ */

void consumption_platform_log(int level, const char* message) {
    /* NXP logging - send to LPUART */

    const char* level_str[] = {"ERROR", "WARN", "INFO", "DEBUG"};

    if (level >= 0 && level <= 3) {
        /* Format log message */
        int len = snprintf(log_buffer, LOG_BUFFER_SIZE,
                          "[%s] %s\r\n", level_str[level], message);

        if (len > 0 && len < LOG_BUFFER_SIZE) {
            /* Send to LPUART */
            LPUART_WriteBlocking(LPUART1, (uint8_t*)log_buffer, (size_t)len);
        }
    }

    /* Optionally store critical logs in SNVS */
    if (level == 0) {  /* Error level */
        /* Store in SNVS general purpose registers */
    }
}

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

void* consumption_platform_malloc(size_t size) {
    /* Use FreeRTOS pvPortMalloc or standard malloc */
#ifdef USE_FREERTOS
    return pvPortMalloc(size);
#else
    return malloc(size);
#endif
}

void consumption_platform_free(void* ptr) {
#ifdef USE_FREERTOS
    vPortFree(ptr);
#else
    free(ptr);
#endif
}

/* ============================================================================
 * THREADING
 * ============================================================================ */

void consumption_platform_enter_critical(void) {
    /* Enter critical section */
#ifdef USE_FREERTOS
    taskENTER_CRITICAL();
#else
    __disable_irq();
#endif
}

void consumption_platform_exit_critical(void) {
    /* Exit critical section */
#ifdef USE_FREERTOS
    taskEXIT_CRITICAL();
#else
    __enable_irq();
#endif
}

/* ============================================================================
 * PLATFORM INIT/DEINIT
 * ============================================================================ */

bool consumption_platform_init(void) {
    status_t status;

    /* Initialize Flash driver */
    status = FLASH_Init(&s_flashDriver);
    if (status != kStatus_Success) {
        return false;
    }

    /* Initialize SNVS/ RTC if not already done */
    /* This should be done in board initialization */

    /* Initialize LPUART for logging if needed */
    /* Should be done in board initialization */

    return true;
}

void consumption_platform_deinit(void) {
    /* Deinitialize peripherals if needed */
    /* Usually not needed for NXP */
}

/* ============================================================================
 * NXP SPECIFIC HELPERS
 * ============================================================================ */

/**
 * @brief Initialize NXP Flash for storage operations
 * Call this once during system initialization
 */
void consumption_nxp_flash_init(void) {
    /* Configure flash driver */
    FLASH_GetDefaultConfig(&s_flashDriver);
    FLASH_Init(&s_flashDriver);
}

/**
 * @brief Get system uptime in milliseconds (alternative timestamp)
 * @return Uptime in milliseconds
 */
uint32_t consumption_nxp_get_uptime_ms(void) {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/**
 * @brief Store critical error in SNVS
 * @param error_code Error code to store
 */
void consumption_nxp_store_critical_error(uint16_t error_code) {
    /* Store in SNVS general purpose register */
    SNVS_LP_RTC_Write_GP_REG(SNVS_LP_RTC, 0, (uint32_t)error_code);
}

/**
 * @brief Read critical error from SNVS
 * @return Stored error code
 */
uint16_t consumption_nxp_read_critical_error(void) {
    return (uint16_t)SNVS_LP_RTC_Read_GP_REG(SNVS_LP_RTC, 0);
}

/**
 * @brief Configure network interface for consumption module
 * @param network_config Network configuration structure
 * @return true on success
 */
bool consumption_nxp_network_init(void* network_config) {
    /* Initialize network stack (LWIP/FreeRTOS+TCP) */
    /* Configure IP, gateway, DNS, etc. */

    (void)network_config;  /* Placeholder */
    return true;
}
