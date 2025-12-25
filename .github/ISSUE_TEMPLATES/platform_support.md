---
name: 🔧 Platform Support
about: Request support for a new platform or report platform-specific issues
title: "[PLATFORM] Add support for [Platform Name]"
labels: platform, enhancement
assignees: ''

---

## 🔧 Platform Support Request

### Platform Details
- **Platform Name:** [e.g. ESP32, PIC32, Arduino, Custom MCU]
- **MCU Family:** [e.g. ESP32-S3, PIC32MZ, ATMega2560]
- **Architecture:** [ARM Cortex-M, MIPS, AVR, RISC-V, etc.]
- **Vendor:** [Espressif, Microchip, Atmel, Custom]

### Technical Specifications
- **CPU:** [e.g. ARM Cortex-M4, MIPS M4K, AVR 8-bit]
- **Clock Speed:** [MHz]
- **RAM:** [KB]
- **Flash/Storage:** [KB, type]
- **Peripherals:** [RTC, Flash, UART, Network, etc.]

### Requirements Analysis

#### ✅ Available Resources
What hardware resources are available on this platform?

- [ ] Real-time clock (RTC) or system time
- [ ] Non-volatile storage (Flash, EEPROM, FRAM)
- [ ] UART/Serial for logging
- [ ] Network interface (Ethernet, WiFi, Cellular)
- [ ] External libraries (FreeRTOS, LwIP, etc.)

#### ⚠️ Missing Resources
What resources might need workarounds?

- [ ] No RTC (use uptime counter + offset)
- [ ] Limited storage (smaller buffer sizes)
- [ ] No network (local-only operation)
- [ ] Limited RAM (optimize memory usage)

### Implementation Plan

#### Phase 1: Core Functionality
- [ ] Platform abstraction layer (`consumption_platform_[name].c`)
- [ ] Time functions (timestamp)
- [ ] Storage functions (read/write)
- [ ] Logging functions
- [ ] Memory management
- [ ] Basic testing

#### Phase 2: Extended Features
- [ ] Network support (if available)
- [ ] Platform-specific optimizations
- [ ] Advanced testing
- [ ] Performance benchmarking

#### Phase 3: Documentation & Examples
- [ ] Platform-specific documentation
- [ ] Integration examples
- [ ] Build system updates
- [ ] CI/CD integration

### Existing Similar Platforms
Which existing platforms are most similar?

- [ ] STM32 (HAL-based)
- [ ] NXP (MCUXpresso SDK)
- [ ] Linux (POSIX)
- [ ] Other: ________

### Community Interest
- **Estimated Users:** How many developers might use this platform?
- **Use Cases:** Specific applications (vending machines, IoT devices, etc.)
- **Priority:** [Low, Medium, High, Critical]

### Contribution Willingness
Are you willing to contribute to the implementation?

- [ ] I can provide platform-specific code
- [ ] I can test on real hardware
- [ ] I can help with documentation
- [ ] I need full implementation from maintainers
- [ ] Other: ________

### Additional Context
- **Hardware Documentation:** Links to datasheets, reference manuals
- **Development Tools:** IDE, compiler, debugger
- **Existing Code:** Do you have existing integration attempts?
- **Timeline:** When do you need this platform supported?

### Checklist
- [ ] Platform specifications provided
- [ ] Hardware resources assessed
- [ ] Implementation complexity evaluated
- [ ] Testing strategy outlined
- [ ] Community interest estimated

---

**Note:** Platform support depends on community interest and maintainer availability. Popular platforms are prioritized.
