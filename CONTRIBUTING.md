# Contributing to Consumption Counter Module

[![GitHub](https://img.shields.io/badge/GitHub-Repository-blue?logo=github)](https://github.com/Kavis1/consumption-counter)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/Kavis1/consumption-counter/pulls)
[![Issues](https://img.shields.io/badge/Issues-welcome-blue.svg)](https://github.com/Kavis1/consumption-counter/issues)

> **Note:** This project is hosted on GitHub. All contributions must be made through the [GitHub repository](https://github.com/Kavis1/consumption-counter).

We welcome contributions from the community! This document provides guidelines for contributing to the Consumption Counter Module project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)
- [Submitting Changes](#submitting-changes)
- [Reporting Issues](#reporting-issues)
- [Community](#community)

## Code of Conduct

This project follows a code of conduct to ensure a welcoming environment for all contributors. By participating, you agree to:

- Be respectful and inclusive
- Focus on constructive feedback
- Accept responsibility for mistakes
- Show empathy towards other contributors
- Help create a positive community

## Getting Started

### Prerequisites

Before contributing, ensure you have:

- **C99 compatible compiler** (GCC, Clang, MSVC)
- **Make** build system
- **Git** version control
- **Optional:** libcurl and libmosquitto for network features

### Quick Setup

```bash
# Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/consumption-counter.git
cd consumption-counter

# Build the project
make

# Run tests
make test

# Run demo
make run-demo
```

## Development Setup

### Building for Development

```bash
# Build with debug symbols
make CFLAGS="-g -O0"

# Build with network support
make USE_NETWORK=1

# Build for specific platform
make PLATFORM=linux
make PLATFORM=stm32
```

### Development Tools

We recommend using these tools for development:

- **Compiler**: GCC 7+ or Clang 6+
- **Build System**: GNU Make 4+
- **Code Analysis**: clang-tidy, cppcheck
- **Testing**: Unity framework (included)
- **Documentation**: Doxygen (optional)

### IDE Setup

#### VS Code
```json
{
  "C_Cpp.default.compilerPath": "gcc",
  "C_Cpp.default.cStandard": "c99",
  "C_Cpp.default.includePath": ["include"],
  "C_Cpp.default.defines": ["USE_CURL", "USE_MOSQUITTO"]
}
```

#### CLion
- Import as Makefile project
- Set C99 standard in CMakeLists.txt

## Development Workflow

### 1. Choose an Issue

- Check [open issues](https://github.com/Kavis1/consumption-counter/issues) on GitHub
- Look for issues labeled `good first issue` or `help wanted`
- Comment on the issue to indicate you're working on it

### 2. Create a Branch

```bash
# Create and switch to a new branch
git checkout -b feature/your-feature-name
# or
git checkout -b fix/issue-number-description
```

### 3. Make Changes

- Follow the [coding standards](#coding-standards)
- Write tests for new functionality
- Update documentation as needed
- Ensure all tests pass

### 4. Test Your Changes

```bash
# Run unit tests
make tests && ./build/bin/consumption_tests

# Run integration tests
make run-demo

# Run static analysis
make lint
```

### 5. Commit Your Changes

```bash
# Stage your changes
git add .

# Commit with descriptive message
git commit -m "feat: add new feature description

- What was changed
- Why it was changed
- How it was tested
"

# Follow conventional commit format:
# feat: new feature
# fix: bug fix
# docs: documentation
# style: formatting
# refactor: code restructuring
# test: testing
# chore: maintenance
```

### 6. Push and Create Pull Request

```bash
# Push your branch
git push origin feature/your-feature-name

# Create a pull request on GitHub
# Fill out the PR template with details
```

## Coding Standards

### C Code Style

We follow a consistent C coding style:

```c
// File header
/**
 * @file filename.c
 * @brief Brief description
 */

// Includes
#include "local_header.h"
#include <system_header.h>

// Macros and constants
#define CONSTANT_NAME 42
#define MAX_BUFFER_SIZE 1024

// Type definitions
typedef struct {
    int field1;
    char* field2;
} my_struct_t;

// Function prototypes (in header files)
return_type function_name(param_type param_name);

// Functions
/**
 * @brief Function description
 * @param param1 Description of parameter
 * @return Description of return value
 */
static return_type function_name(param_type param_name) {
    // Local variables at top
    int local_var = 0;

    // Early returns for error conditions
    if (param_name == NULL) {
        return ERROR_VALUE;
    }

    // Main logic
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (condition) {
            // Handle condition
        }
    }

    return SUCCESS_VALUE;
}
```

### Naming Conventions

- **Files**: `snake_case.c` or `snake_case.h`
- **Functions**: `snake_case()`
- **Variables**: `snake_case`
- **Constants**: `UPPER_SNAKE_CASE`
- **Types**: `snake_case_t`
- **Macros**: `UPPER_SNAKE_CASE`

### Code Quality Requirements

- **No compiler warnings** with `-Wall -Wextra -Werror`
- **No memory leaks** (use valgrind for checking)
- **Thread-safe** where applicable
- **Input validation** on all public APIs
- **Error handling** with appropriate error codes
- **Documentation** for all public APIs

### Platform-Specific Code

```c
// Use conditional compilation for platform differences
#ifdef PLATFORM_STM32
    // STM32 specific code
    HAL_FLASH_Read(...);
#elif defined(PLATFORM_NXP)
    // NXP specific code
    FLASH_Read(...);
#else
    // POSIX fallback
    fread(...);
#endif
```

## Testing

### Unit Tests

We use a simple test framework. Add tests to `tests/test_*.c`:

```c
#include "consumption.h"
#include "../unity/unity.h"

// Test functions
void test_initialization(void) {
    // Arrange
    consumption_config_t config = {0};

    // Act
    consumption_error_t result = consumption_init(&config);

    // Assert
    TEST_ASSERT_EQUAL(CONSUMPTION_SUCCESS, result);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_initialization);
    // Add more tests...

    return UNITY_END();
}
```

### Running Tests

```bash
# Build and run all tests
make tests && ./build/bin/consumption_tests

# Run specific test
./build/bin/consumption_tests TestGroupName

# Run with verbose output
./build/bin/consumption_tests -v
```

### Test Coverage

Aim for high test coverage:

- **Unit tests** for all functions
- **Integration tests** for workflows
- **Platform tests** for different targets
- **Edge case testing** for error conditions

### Performance Testing

```bash
# Benchmark critical functions
time ./build/bin/benchmark_test

# Memory usage testing
valgrind --leak-check=full ./build/bin/consumption_tests
```

## Documentation

### Code Documentation

Use Doxygen-style comments for all public APIs:

```c
/**
 * @brief Brief description of function
 *
 * Detailed description of what the function does,
 * including any side effects or important notes.
 *
 * @param[in] input_param Description of input parameter
 * @param[out] output_param Description of output parameter
 * @param[in,out] inout_param Description of input/output parameter
 *
 * @return Description of return value
 *
 * @note Any important notes
 * @warning Any warnings
 * @see Related functions or documentation
 */
return_type function_name(param_type input_param);
```

### Documentation Updates

When making changes:

1. **Update API docs** if function signatures change
2. **Update examples** if usage patterns change
3. **Update README** for new features
4. **Update CHANGELOG** for all changes

### Building Documentation

```bash
# Generate API documentation (requires Doxygen)
doxygen docs/Doxyfile

# View generated docs
open docs/html/index.html
```

## Submitting Changes

### Pull Request Process

1. **Ensure tests pass** locally
2. **Update documentation** as needed
3. **Write clear commit messages**
4. **Create descriptive PR title**
5. **Fill out PR template** completely

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests pass
- [ ] Manual testing performed

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Tests pass
- [ ] No new warnings
```

### Review Process

- **Automated checks** run on PR
- **Code review** by maintainers
- **CI/CD pipeline** validates changes
- **Squash and merge** for clean history

## Reporting Issues

### Bug Reports

Use the [Bug Report template](https://github.com/Kavis1/consumption-counter/issues/new?template=bug_report.md) for reporting bugs. The template guides you through providing:

- Clear bug description and reproduction steps
- Environment details (OS, platform, compiler)
- Code examples and error logs
- Expected vs actual behavior

### Feature Requests

Use the [Feature Request template](https://github.com/Kavis1/consumption-counter/issues/new?template=feature_request.md) for suggesting new features. The template helps you describe:

- Problem statement and use cases
- Proposed solution with implementation details
- Alternative approaches considered
- Priority and benefits assessment

### Other Issue Types

- **[Documentation Issues](https://github.com/Kavis1/consumption-counter/issues/new?template=documentation.md)**: Report missing, incorrect, or unclear documentation
- **[Platform Support](https://github.com/Kavis1/consumption-counter/issues/new?template=platform_support.md)**: Request support for new microcontroller platforms
- **[Support Requests](https://github.com/Kavis1/consumption-counter/issues/new?template=support.md)**: Get help with integration or usage questions

### General Issues

You can also create [blank issues](https://github.com/Kavis1/consumption-counter/issues/new) for questions, discussions, or issues that don't fit the templates above.

## Community

### Getting Help

- **📧 Email**: support@vendor.com
- **💬 Discussions**: [GitHub Discussions](https://github.com/Kavis1/consumption-counter/discussions)
- **📖 Documentation**: [Wiki](https://github.com/Kavis1/consumption-counter/wiki)
- **🎯 Issues**: [GitHub Issues](https://github.com/Kavis1/consumption-counter/issues)

### Recognition

Contributors are recognized through:

- **GitHub Contributors** list
- **CHANGELOG.md** mentions
- **Release notes** credits
- **Community recognition** posts

### License

By contributing, you agree that your contributions will be licensed under the same license as the project (MIT License).

---

## Thank You!

Your contributions help make the Consumption Counter Module better for everyone. We appreciate your time and effort!

**[🚀 Get Started](../README.md)** | **[📖 Integration Guide](docs/integration_guide.md)** | **[📚 API Reference](docs/api_reference.md)** | **[🧪 Live Demo](../examples/)**
