# reg_api

A modern C++17 Windows Registry API wrapper providing type-safe, RAII-based registry operations.

## Features

- ✅ **Type-Safe Operations** - Template-based numeric type handling
- ✅ **RAII Design** - Automatic registry handle management
- ✅ **String & Numeric Types** - Read/write strings, integers, floats, doubles
- ✅ **Object Serialization** - Store POD objects with Base64 encoding
- ✅ **Pointer Storage** - Debug/IPC pointer storage with process ID validation
- ✅ **Default Values** - All read operations support default values
- ✅ **Auto Key Creation** - Automatically creates keys if they don't exist

## Quick Start

### Build and Run

On Windows, use Visual Studio 2022 command prompt:

```bash
make run
```

### Basic Usage

```cpp
#include "reg_api.h"

// Create registry handler
reg_api reg;

// Open or create a registry key
if (reg.chroot("Software\\MyApplication")) {
    // Write operations
    reg.write_string("app_name", "My Application");
    reg.write_number("version", 123);
    reg.write_number("pi", 3.14159);
    
    // Read operations with defaults
    std::string name = reg.read_string("app_name", "Default");
    int version = reg.read_number<int>("version", 0);
    double pi = reg.read_number<double>("pi", 0.0);
    
    // Check if value exists
    if (reg.value_exists("install_date")) {
        // Handle existing value
    }
    
    // Clean up
    reg.close();
}
```

### Advanced Features

#### Object Serialization

```cpp
struct Config {
    int width;
    int height;
    bool fullscreen;
};

Config config = {1920, 1080, true};

// Write struct to registry
reg.write_obj("config", config);

// Read struct from registry
Config loaded = reg.read_obj<Config>("config");
```

#### Base64 Encoding

```cpp
std::string data = "Hello, World!";
std::string encoded = reg.b64(data);
std::string decoded = reg.b64_d(encoded);
```

## API Reference

### Constructor
```cpp
reg_api(HKEY h_root_key = HKEY_CURRENT_USER)
```

### Key Operations
- `bool chroot(const std::string& key_path)` - Open or create registry key
- `void close()` - Close current key
- `bool value_exists(const std::string& value_name)` - Check if value exists
- `bool delete_value(const std::string& value_name)` - Delete a value

### Read Operations
- `std::string read_string(name, default)` - Read string value
- `T read_number<T>(name, default)` - Read numeric value
- `T read_obj<T>(name)` - Read serialized object
- `T* read_pointer<T>(name, default)` - Read pointer (same process only)

### Write Operations
- `bool write_string(name, value)` - Write string value
- `bool write_number<T>(name, value)` - Write numeric value
- `void write_obj<T>(name, value)` - Write serialized object
- `bool write_pointer<T>(name, ptr)` - Write pointer (debugging only)

### Utility Functions
- `std::string b64(const std::string& in)` - Encode to Base64
- `std::string b64_d(const std::string& in)` - Decode from Base64

## Example Output

```text
App Name: My Registry Test App
Version: 123
Pi: 3.14159
Price: 99.99
Non-existent: Default Value
Non-existent num: 42
Pointer to int vector: 11,22,33,44,55,
install_date does not exist in reg.
Read xyz: x=11, y=22, z=33
```

## Build System

### Makefile Targets

- `make all` - Build the project
- `make run` - Build and run the example
- `make clean` - Clean build artifacts

### Build Requirements

- Windows OS
- Visual Studio 2022 (or compatible MSVC compiler)
- C++17 or later

## Project Structure

```
reg_api/
├── reg_api.h          # Header file with class definition
├── reg_api.cpp        # Implementation file
├── main.cpp           # Example usage
├── Makefile           # Build configuration
└── README.md          # This file
```

## Important Notes

### Thread Safety
⚠️ This class is **NOT thread-safe**. Use external synchronization if accessing from multiple threads.

### Pointer Operations
⚠️ The `write_pointer`/`read_pointer` functions should only be used for:
- Debugging purposes
- Inter-thread communication within the same process instance

**Do not** rely on pointer values across process restarts due to:
- Address Space Layout Randomization (ASLR)
- Memory address reuse
- Process isolation

### Numeric Storage
All numeric values are stored as formatted strings in the registry to avoid Windows Registry type limitations and ensure precision.

## Development Advice

Looking to improve this project? Check out:
- [DEVELOPMENT_ADVICE.md](DEVELOPMENT_ADVICE.md) - Comprehensive development recommendations
- [QUICK_START_IMPROVEMENTS.md](QUICK_START_IMPROVEMENTS.md) - Quick wins you can implement in ~1.5 hours

Key areas for improvement:
1. Add unit testing infrastructure
2. Implement comprehensive error handling
3. Add CMake build system
4. Improve documentation
5. Add CI/CD pipeline

## License

MIT License - See [LICENSE](LICENSE) file for details

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

