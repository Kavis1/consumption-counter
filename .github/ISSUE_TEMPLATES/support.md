---
name: 🆘 Support Request
about: Get help with using or integrating the Consumption Counter Module
title: "[SUPPORT] Brief description of your question"
labels: question, support
assignees: ''

---

## 🆘 Support Request

### Question Summary
Brief summary of your question or the help you need.

### Context
Provide context about what you're trying to accomplish:

- **Project Stage:** [Planning, Development, Testing, Production]
- **Integration Type:** [New project, Existing codebase, Migration]
- **Timeline:** [ASAP, This week, This month, No rush]

### Environment Details
- **OS:** [e.g. Linux Ubuntu 20.04, Windows 10]
- **Platform:** [e.g. STM32F407, NXP i.MX RT1060, Raspberry Pi]
- **Compiler:** [e.g. GCC 9.3, ARM GCC 10.3, Clang 12]
- **Build System:** [Make, CMake, IDE]
- **Module Version:** [e.g. 1.0.0, latest]
- **Network Setup:** [None, libcurl, libmosquitto, custom]

### What Have You Tried?
Describe what you've already attempted:

1. **Step 1:** What you did and what happened
2. **Step 2:** What you tried next and the result
3. **Step 3:** Any other approaches

### Code Example
If applicable, share relevant code snippets:

```c
// Your integration code
consumption_config_t config = {
    .machine_id = YOUR_ID,
    // ... your configuration
};

consumption_error_t result = consumption_init(&config);
// What error are you getting?
```

### Expected vs Actual Behavior
- **Expected:** What you think should happen
- **Actual:** What's actually happening

### Documentation Reference
Which documentation have you consulted?

- [ ] README.md
- [ ] Integration Guide (docs/integration_guide.md)
- [ ] API Reference (docs/api_reference.md)
- [ ] Examples
- [ ] Other: ________

### Additional Resources
Have you checked these resources?

- [ ] GitHub Issues (similar problems)
- [ ] GitHub Discussions
- [ ] Stack Overflow
- [ ] Vendor documentation
- [ ] Other forums/communities

### Urgency
How urgent is your need for help?

- [ ] Blocking development
- [ ] Significant delay
- [ ] Minor inconvenience
- [ ] Just curious

### Contact Information
If you'd prefer direct communication:

- **Email:** [your email, optional]
- **Company:** [if applicable]
- **Time Zone:** [for scheduling calls]

---

**Note:** For urgent security issues, please email security@vendor.com instead of creating a public issue.

**Quick Help:** Check the [Integration Guide](docs/integration_guide.md) and [Examples](../examples/) first!
