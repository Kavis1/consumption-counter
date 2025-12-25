# Consumption Counter Module Integration Guide

## For Vending Machine Manufacturers

This guide will help you integrate the consumption tracking module into your vending machine without changing core functionality.

## 📋 Prerequisites

### Technical Requirements
- **C99 compiler** (GCC, Clang, ARMCC, etc.)
- **Non-volatile memory** (Flash ≥ 8KB, EEPROM, or file system)
- **Real-time clock** (RTC) or ability to get Unix timestamp
- **Optional:** Network subsystem for external API

### System Requirements
- **RAM:** ≥ 4KB + event buffer
- **Flash/Storage:** ≥ 8KB for code + data storage
- **CPU:** Any architecture with C99 support

## 🚀 Quick Integration (5 minutes)

### Step 1: Add Files to Project

Copy to your project:
```
your_project/
├── consumption.h
├── consumption_platform.h
└── consumption.c
```

### Step 2: Implement Platform

Create `consumption_platform.c` file with implementation for your platform:

```c
#include "consumption_platform.h"

// Example for STM32 + FreeRTOS
uint32_t consumption_platform_get_timestamp(void) {
    return (uint32_t)time(NULL); // Or HAL_GetTick()/1000 + offset
}

bool consumption_platform_storage_read(void* data, size_t size) {
    // Read from Flash sector
    return (HAL_FLASH_Read(data, size) == HAL_OK);
}

bool consumption_platform_storage_write(const void* data, size_t size) {
    // Write to Flash with wear-leveling
    return (HAL_FLASH_Write(data, size) == HAL_OK);
}

bool consumption_platform_network_send(const char* endpoint,
                                     const char* data, size_t data_len) {
    // Use your network library (LWIP, etc.)
    return your_network_post(endpoint, data, data_len);
}

void consumption_platform_log(int level, const char* message) {
    // Integrate with your logging system
    your_log_printf("%s", message);
}
```

### Step 3: Initialize Module

In your vending machine initialization code:

```c
#include "consumption.h"

void vending_machine_init(void) {
    // Your existing initialization...

    // Add: Initialize consumption tracking
    consumption_config_t config = {
        .machine_id = YOUR_MACHINE_ID,     // Unique machine ID
        .enable_external_api = false,      // Disabled by default
        .ring_buffer_size = 1000,          // Adjust based on needs
        .aggregation_interval = 3600,      // 1 hour
    };

    consumption_error_t result = consumption_init(&config);
    if (result != CONSUMPTION_SUCCESS) {
        // Log error but continue operation
        your_log_error("Consumption init failed: %s",
                      consumption_error_string(result));
    }

    // Optional: Register boot event
    consumption_on_boot();
}
```

### Step 4: Add Dispensing Tracking

Find your beverage dispensing function and add one line:

```c
// Before:
void dispense_drink(uint8_t product_id) {
    activate_pump(product_id);
    wait_for_completion();
    update_inventory(product_id);
}

// After:
void dispense_drink(uint8_t product_id) {
    activate_pump(product_id);
    wait_for_completion();
    update_inventory(product_id);

    // ADD: Consumption tracking
    consumption_on_dispense(YOUR_MACHINE_ID, product_id);
}
```

**Critical:** Call `consumption_on_dispense()` ONLY after successful dispensing!

### Step 5: Deinitialize

When shutting down the vending machine:

```c
void vending_machine_shutdown(void) {
    // Your existing deinitialization...

    // Add: Proper consumption tracking shutdown
    consumption_on_shutdown();
    consumption_deinit();
}
```

## ⚙️ Configuration

### Main Parameters

```c
consumption_config_t config = {
    .machine_id = 12345,              // Required: Unique machine ID
    .enable_external_api = false,     // Disabled by default
    .ring_buffer_size = 1000,         // 100-10000, depends on memory
    .aggregation_interval = 3600,     // Seconds between data sends
    .api_endpoint = "https://your-api.com/consumption",
    .api_key = "your-secret-key",
    .max_retry_attempts = 3,
};
```

### Configuration Recommendations

| Parameter | Small Machine | Medium Machine | Large Machine |
|-----------|---------------|----------------|---------------|
| ring_buffer_size | 500 | 1000 | 5000 |
| aggregation_interval | 7200 (2h) | 3600 (1h) | 1800 (30min) |

## 🌐 External API (Optional)

### Enabling API

```c
consumption_config_t config = {
    // ... other parameters
    .enable_external_api = true,
    .api_endpoint = "https://api.yourcompany.com/vending/consumption",
    .api_key = "your-api-key-here",
};
```

### Data Format

The module sends JSON with aggregated data:

```json
{
  "machine_id": 12345,
  "period_start": 1735152000,
  "period_end": 1735155600,
  "total_events": 150,
  "products": {
    "1": 45,
    "2": 38,
    "3": 67
  }
}
```

### API Testing

```bash
# Test data sending
curl -X POST https://your-api.com/consumption \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer your-api-key" \
  -d '{"machine_id":12345,"total_events":10}'
```

## 🔧 Advanced Integration

### Multi-Product Machines

```c
void dispense_product(uint8_t product_type, uint8_t product_id) {
    // product_type: 1=drink, 2=snack, 3=coffee
    // product_id: specific product ID

    // Successful dispensing...

    // Track with combined ID
    uint8_t combined_id = (product_type << 6) | product_id;
    consumption_on_dispense(machine_id, combined_id);
}
```

### Error Handling

```c
consumption_error_t result = consumption_on_dispense(machine_id, product_id);
if (result != CONSUMPTION_SUCCESS) {
    // Log but DO NOT stop the machine
    your_log_warning("Consumption tracking failed: %s",
                    consumption_error_string(result));
    // Continue normal machine operation
}
```

### Monitoring and Statistics

```c
// Periodically check status
void check_consumption_status(void) {
    uint32_t total_events, buffered_events, last_sync;

    if (consumption_get_stats(&total_events, &buffered_events, &last_sync)
        == CONSUMPTION_SUCCESS) {

        your_log_info("Consumption: %u total, %u buffered, last sync: %u",
                     total_events, buffered_events, last_sync);
    }
}
```

## 🧪 Integration Testing

### Unit Tests
```bash
# Run built-in tests
make tests && ./build/bin/consumption_tests
```

### Integration Tests
```bash
# Run demo application
make run-demo
```

### Manual Testing
1. Initialize machine
2. Make several "purchases"
3. Check statistics via `consumption_get_stats()`
4. With API enabled, verify data sending

### Load Testing
- 1000+ dispensing events
- Buffer overflow
- Network absence (with API enabled)
- System restarts

## 🐛 Troubleshooting

### Issue: Module Won't Initialize
**Solution:**
- Check `machine_id > 0`
- Check `ring_buffer_size` in range 1-10000
- Check memory availability for buffer

### Issue: Events Not Being Recorded
**Solution:**
- Ensure `consumption_on_dispense()` is called AFTER successful dispensing
- Check that `machine_id` matches configuration
- Check `product_id` in range 1-255

### Issue: Data Not Being Sent
**Solution:**
- Check `enable_external_api = true`
- Check network availability
- Check correct URL and API key
- Check logs for network errors

### Issue: High Memory Usage
**Solution:**
- Reduce `ring_buffer_size`
- Optimize platform functions
- Check for memory leaks in your implementation

## 📊 Performance

### Typical Metrics
- **Initialization:** < 50ms
- **Event Registration:** < 2ms
- **API Send:** < 500ms (depends on network)
- **Memory:** 4KB + (event_size × buffer)

### Optimizations
- Use static buffer instead of dynamic
- Minimize calls to `consumption_platform_storage_write()`
- Cache timestamp for batch processing

## 🔄 Module Updates

### Safe Update
1. Deinitialize module: `consumption_deinit()`
2. Update module code
3. Reinitialize: `consumption_init()`
4. Data is preserved automatically

### Backward Compatibility
- New versions maintain API compatibility
- Configuration automatically migrates
- Old data remains accessible

## 📞 Support

If you encounter integration problems:

1. Check module logs
2. Run tests: `make run-tests`
3. Gather diagnostic information
4. Contact technical support

## ✅ Integration Checklist

- [ ] Module files added to project
- [ ] Platform functions implemented
- [ ] Module initializes on startup
- [ ] `consumption_on_dispense()` called after dispensing
- [ ] Module deinitializes on shutdown
- [ ] Testing completed successfully
- [ ] API tested (if enabled)
- [ ] Documentation updated

---

**Integration Checklist Version:** 1.0  
**Last Updated:** December 2024
