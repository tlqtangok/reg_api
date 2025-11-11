# Best Practices and Common Patterns

This document demonstrates recommended usage patterns and best practices for the reg_api library.

## Table of Contents
1. [Error Handling Patterns](#error-handling-patterns)
2. [Resource Management](#resource-management)
3. [Type Safety](#type-safety)
4. [Performance Tips](#performance-tips)
5. [Security Considerations](#security-considerations)
6. [Common Pitfalls](#common-pitfalls)

---

## Error Handling Patterns

### ✅ Good: Always Check Return Values

```cpp
reg_api reg;

if (!reg.chroot("Software\\MyApp")) {
    std::cerr << "Failed to open registry key" << std::endl;
    // Handle error appropriately
    return false;
}

if (!reg.write_string("version", "1.0.0")) {
    std::cerr << "Failed to write version" << std::endl;
    // Handle error
}
```

### ✅ Better: Use Error Codes (After implementing error reporting)

```cpp
reg_api reg;

if (!reg.chroot("Software\\MyApp")) {
    std::cerr << "Failed to open registry: " 
              << reg.get_last_error_message() 
              << " (Code: " << reg.get_last_error() << ")" << std::endl;
    return false;
}
```

### ✅ Best: Use RAII and Exceptions for Critical Operations

```cpp
class RegistryGuard {
public:
    RegistryGuard(reg_api& reg, const std::string& key_path)
        : m_reg(reg) {
        if (!m_reg.chroot(key_path)) {
            throw std::runtime_error("Failed to open registry key: " + key_path);
        }
    }
    
    ~RegistryGuard() {
        m_reg.close();
    }
    
    // Prevent copying
    RegistryGuard(const RegistryGuard&) = delete;
    RegistryGuard& operator=(const RegistryGuard&) = delete;
    
private:
    reg_api& m_reg;
};

// Usage
try {
    reg_api reg;
    RegistryGuard guard(reg, "Software\\MyApp");
    
    // Use reg object - automatically closed on scope exit
    reg.write_string("name", "value");
    
} catch (const std::exception& e) {
    std::cerr << "Registry error: " << e.what() << std::endl;
}
```

---

## Resource Management

### ✅ Good: Manual Close

```cpp
void configure_app() {
    reg_api reg;
    
    if (reg.chroot("Software\\MyApp")) {
        reg.write_string("config", "value");
        reg.close();  // Explicit close
    }
}
```

### ✅ Better: Automatic Close via RAII

```cpp
void configure_app() {
    reg_api reg;
    
    if (reg.chroot("Software\\MyApp")) {
        reg.write_string("config", "value");
    }
    // Automatically closed by destructor
}
```

### ✅ Best: Scoped Lifetime

```cpp
void configure_app() {
    {
        reg_api reg;
        if (reg.chroot("Software\\MyApp")) {
            reg.write_string("config", "value");
        }
    }  // reg destroyed here, key closed
    
    // Do other work without holding registry key open
}
```

---

## Type Safety

### ✅ Good: Use Appropriate Types

```cpp
reg_api reg;
reg.chroot("Software\\MyApp");

// Use appropriate types for data
int count = 42;
double price = 19.99;
std::string name = "Product";

reg.write_number("count", count);
reg.write_number("price", price);
reg.write_string("name", name);

// Read with correct types
int loaded_count = reg.read_number<int>("count", 0);
double loaded_price = reg.read_number<double>("price", 0.0);
std::string loaded_name = reg.read_string("name", "");
```

### ❌ Bad: Type Mismatches

```cpp
// Don't write as one type and read as another
reg.write_number("value", 3.14159);  // double
int val = reg.read_number<int>("value", 0);  // Read as int - loses precision!
```

### ✅ Good: Validate Types with Static Assertions

```cpp
template<typename T>
void write_config_value(reg_api& reg, const std::string& name, const T& value) {
    static_assert(std::is_arithmetic<T>::value || std::is_same<T, std::string>::value,
                  "Value must be numeric or string type");
    
    if constexpr (std::is_arithmetic<T>::value) {
        reg.write_number(name, value);
    } else {
        reg.write_string(name, value);
    }
}
```

---

## Performance Tips

### ✅ Minimize Registry Operations

```cpp
// ❌ Bad: Open/close for each operation
void update_settings(const std::vector<std::string>& values) {
    for (const auto& val : values) {
        reg_api reg;
        reg.chroot("Software\\MyApp");
        reg.write_string("key", val);
        reg.close();
    }
}

// ✅ Good: Keep key open for batch operations
void update_settings(const std::vector<std::string>& values) {
    reg_api reg;
    if (reg.chroot("Software\\MyApp")) {
        for (const auto& val : values) {
            reg.write_string("key", val);
        }
    }
}
```

### ✅ Use Default Values Wisely

```cpp
// ✅ Good: Provide sensible defaults
struct AppConfig {
    std::string name;
    int width;
    int height;
    bool fullscreen;
    
    static AppConfig load(reg_api& reg) {
        AppConfig config;
        config.name = reg.read_string("app_name", "My Application");
        config.width = reg.read_number<int>("width", 1920);
        config.height = reg.read_number<int>("height", 1080);
        config.fullscreen = reg.read_number<int>("fullscreen", 0) != 0;
        return config;
    }
    
    void save(reg_api& reg) const {
        reg.write_string("app_name", name);
        reg.write_number("width", width);
        reg.write_number("height", height);
        reg.write_number("fullscreen", fullscreen ? 1 : 0);
    }
};
```

### ✅ Cache Frequently-Used Values

```cpp
class ConfigCache {
public:
    ConfigCache(const std::string& reg_path) : m_reg_path(reg_path) {
        reload();
    }
    
    void reload() {
        reg_api reg;
        if (reg.chroot(m_reg_path)) {
            m_app_name = reg.read_string("app_name", "Default");
            m_version = reg.read_number<int>("version", 1);
            m_last_update = std::chrono::system_clock::now();
        }
    }
    
    const std::string& app_name() const { return m_app_name; }
    int version() const { return m_version; }
    
    bool needs_reload() const {
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_last_update);
        return elapsed.count() > 60;  // Reload every 60 seconds
    }
    
private:
    std::string m_reg_path;
    std::string m_app_name;
    int m_version;
    std::chrono::system_clock::time_point m_last_update;
};
```

---

## Security Considerations

### ✅ Never Store Sensitive Data in Plain Text

```cpp
// ❌ Bad: Storing passwords in plain text
reg.write_string("password", "MyPassword123");

// ✅ Good: Store hashed passwords or use Windows Credential Manager
// For sensitive data, use Windows Data Protection API (DPAPI)
// or store in Windows Credential Manager instead
```

### ✅ Validate Input Before Writing

```cpp
bool write_validated_port(reg_api& reg, int port) {
    if (port < 1 || port > 65535) {
        std::cerr << "Invalid port number: " << port << std::endl;
        return false;
    }
    
    return reg.write_number("port", port);
}

bool write_validated_path(reg_api& reg, const std::string& path) {
    // Validate path format, length, etc.
    if (path.empty() || path.length() > 260) {
        std::cerr << "Invalid path: " << path << std::endl;
        return false;
    }
    
    return reg.write_string("install_path", path);
}
```

### ⚠️ Pointer Operations Are Dangerous

```cpp
// ❌ Bad: Using pointers for persistent data
std::vector<int> data = {1, 2, 3};
reg.write_pointer("data_ptr", &data);

// ... process restarts ...

// This WILL CRASH - pointer is invalid after restart!
auto* ptr = reg.read_pointer<std::vector<int>>("data_ptr");

// ✅ Good: Serialize data instead
std::vector<int> data = {1, 2, 3};
std::string serialized = serialize_vector(data);  // Implement serialization
reg.write_string("data", serialized);

// Safe to read after restart
std::string loaded = reg.read_string("data");
auto data_restored = deserialize_vector(loaded);
```

### ✅ Use Appropriate Root Keys

```cpp
// ✅ Use HKEY_CURRENT_USER for user-specific settings
reg_api user_reg(HKEY_CURRENT_USER);
user_reg.chroot("Software\\MyApp");
user_reg.write_string("username", "John");

// ✅ Use HKEY_LOCAL_MACHINE for machine-wide settings (requires admin)
// Be careful - affects all users!
reg_api machine_reg(HKEY_LOCAL_MACHINE);
if (machine_reg.chroot("Software\\MyApp")) {
    machine_reg.write_string("install_path", "C:\\Program Files\\MyApp");
}
```

---

## Common Pitfalls

### ❌ Pitfall 1: Not Checking chroot() Return Value

```cpp
// ❌ Bad
reg_api reg;
reg.chroot("Software\\MyApp");
reg.write_string("key", "value");  // May silently fail!

// ✅ Good
reg_api reg;
if (reg.chroot("Software\\MyApp")) {
    reg.write_string("key", "value");
} else {
    // Handle error
}
```

### ❌ Pitfall 2: Forgetting to Close (though RAII helps)

```cpp
// ❌ Bad: Holding registry key open unnecessarily
void long_running_operation() {
    reg_api reg;
    reg.chroot("Software\\MyApp");
    std::string config = reg.read_string("config");
    
    // Long processing...
    expensive_computation(config);
    
    reg.close();  // Key was held open during entire computation!
}

// ✅ Good: Close as soon as possible
void long_running_operation() {
    std::string config;
    {
        reg_api reg;
        if (reg.chroot("Software\\MyApp")) {
            config = reg.read_string("config");
        }
    }  // Key closed immediately
    
    expensive_computation(config);
}
```

### ❌ Pitfall 3: Using Pointers Across Processes

```cpp
// ❌ Bad: Assuming pointers work across process boundaries
// Process A
std::vector<int> data = {1, 2, 3};
reg.write_pointer("shared_data", &data);

// Process B - WILL CRASH!
auto* ptr = reg.read_pointer<std::vector<int>>("shared_data");
// Pointer is from different process - accessing it is undefined behavior!
```

### ❌ Pitfall 4: Thread Safety Issues

```cpp
// ❌ Bad: Sharing reg_api across threads without synchronization
class App {
    reg_api m_reg;
    
    void thread1() {
        m_reg.write_string("key1", "value1");  // Race condition!
    }
    
    void thread2() {
        m_reg.write_string("key2", "value2");  // Race condition!
    }
};

// ✅ Good: Each thread has its own instance
class App {
    void thread1() {
        reg_api reg;
        reg.chroot("Software\\MyApp");
        reg.write_string("key1", "value1");
    }
    
    void thread2() {
        reg_api reg;
        reg.chroot("Software\\MyApp");
        reg.write_string("key2", "value2");
    }
};
```

### ❌ Pitfall 5: Excessive Precision for Floating Point

```cpp
// ❌ Questionable: Expecting exact float representation
float pi = 3.14159265358979323846f;
reg.write_number("pi", pi);
float loaded = reg.read_number<float>("pi", 0.0f);
// May not be exactly equal due to string conversion and floating point precision

// ✅ Good: Use appropriate comparison
const float EPSILON = 0.00001f;
if (std::abs(loaded - pi) < EPSILON) {
    // Values are close enough
}
```

---

## Summary Checklist

When using reg_api, always:

- ✅ Check return values from chroot() and write operations
- ✅ Use appropriate types consistently
- ✅ Provide sensible default values
- ✅ Close registry keys as soon as possible
- ✅ Never store sensitive data in plain text
- ✅ Avoid pointer operations for persistent storage
- ✅ Use separate reg_api instances per thread
- ✅ Validate input before writing
- ✅ Handle errors appropriately
- ✅ Document your registry key structure

---

## Additional Resources

- See [DEVELOPMENT_ADVICE.md](DEVELOPMENT_ADVICE.md) for improvement recommendations
- See [QUICK_START_IMPROVEMENTS.md](QUICK_START_IMPROVEMENTS.md) for quick enhancements
- See [README.md](README.md) for API reference
