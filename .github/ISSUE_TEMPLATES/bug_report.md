---
name: 🐛 Bug Report
about: Create a report to help us improve
title: "[BUG] Brief description of the issue"
labels: bug
assignees: ''

---

## 🐛 Bug Report

### Description
A clear and concise description of what the bug is.

### Steps to Reproduce
Steps to reproduce the behavior:
1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

### Expected Behavior
A clear and concise description of what you expected to happen.

### Actual Behavior
What actually happened instead.

### Environment
- **OS:** [e.g. Linux Ubuntu 20.04, Windows 10, macOS 12.0]
- **Platform:** [e.g. STM32F4, NXP i.MX RT1060, Linux x86_64]
- **Compiler:** [e.g. GCC 9.3.0, ARM GCC 10.3]
- **Build System:** [e.g. Make, CMake]
- **Module Version:** [e.g. 1.0.0]
- **Network Support:** [e.g. Yes (libcurl 7.74), No]

### Code Example
If applicable, provide a minimal code example that reproduces the issue:

```c
// Your code here
consumption_config_t config = {
    .machine_id = 12345,
    // ... other config
};

consumption_error_t result = consumption_init(&config);
// Expected: CONSUMPTION_SUCCESS
// Actual: CONSUMPTION_ERROR_INVALID_CONFIG
```

### Logs and Screenshots
If applicable, add logs, console output, or screenshots to help explain your problem.

### Additional Context
Add any other context about the problem here. This could include:
- Recent changes that might have caused the issue
- Workarounds you've tried
- Related issues or pull requests
- Performance impact (if applicable)

### Checklist
- [ ] I have searched existing issues for similar bugs
- [ ] I have provided a minimal reproducible example
- [ ] I have included relevant environment information
- [ ] I have checked the documentation for potential solutions

---

**Note:** If this is a security-related issue, please email security@vendor.com instead of creating a public issue.
