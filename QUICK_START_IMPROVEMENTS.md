# Quick Start: Implementing Priority Improvements

This guide shows you how to quickly implement the most impactful improvements to the reg_api project.

## 1. Add Basic Unit Testing (30 minutes)

### Step 1: Add Google Test

Download and integrate Google Test, or use Doctest (header-only, easier):

**Option A: Using Doctest (Recommended for quick start)**

1. Download `doctest.h` from https://github.com/doctest/doctest/releases
2. Place it in `tests/doctest.h`

### Step 2: Create Basic Test File

Create `tests/test_basic_operations.cpp`:

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../reg_api.h"

TEST_CASE("String read/write operations") {
    reg_api reg;
    REQUIRE(reg.chroot("Software\\MyApp\\TestKey"));
    
    SUBCASE("Write and read string") {
        REQUIRE(reg.write_string("test_name", "test_value"));
        CHECK(reg.read_string("test_name", "") == "test_value");
    }
    
    SUBCASE("Read non-existent with default") {
        std::string result = reg.read_string("nonexistent", "default");
        CHECK(result == "default");
    }
    
    SUBCASE("Empty string handling") {
        REQUIRE(reg.write_string("empty", ""));
        CHECK(reg.read_string("empty", "default") == "");
    }
    
    // Cleanup
    reg.delete_value("test_name");
    reg.delete_value("empty");
}

TEST_CASE("Numeric type operations") {
    reg_api reg;
    REQUIRE(reg.chroot("Software\\MyApp\\TestKey"));
    
    SUBCASE("Integer operations") {
        reg.write_number("int_val", 42);
        CHECK(reg.read_number<int>("int_val", 0) == 42);
        
        reg.write_number("negative", -123);
        CHECK(reg.read_number<int>("negative", 0) == -123);
    }
    
    SUBCASE("Floating point operations") {
        reg.write_number("pi", 3.14159);
        CHECK(reg.read_number<double>("pi", 0.0) == doctest::Approx(3.14159));
    }
    
    // Cleanup
    reg.delete_value("int_val");
    reg.delete_value("negative");
    reg.delete_value("pi");
}

TEST_CASE("Base64 encoding/decoding") {
    reg_api reg;
    
    std::string original = "Hello, World!";
    std::string encoded = reg.b64(original);
    std::string decoded = reg.b64_d(encoded);
    
    CHECK(decoded == original);
    CHECK(encoded != original);
    CHECK(!encoded.empty());
}

TEST_CASE("Object serialization") {
    struct TestStruct {
        int x;
        int y;
        int z;
    };
    
    reg_api reg;
    REQUIRE(reg.chroot("Software\\MyApp\\TestKey"));
    
    TestStruct original = {10, 20, 30};
    reg.write_obj("test_struct", original);
    
    TestStruct read_back = reg.read_obj<TestStruct>("test_struct");
    
    CHECK(read_back.x == original.x);
    CHECK(read_back.y == original.y);
    CHECK(read_back.z == original.z);
    
    // Cleanup
    reg.delete_value("test_struct");
}
```

### Step 3: Update Makefile

Add to Makefile:
```makefile
# Test target
TEST_SRCS = tests/test_basic_operations.cpp reg_api.cpp
TEST_OBJS = $(BUILD_DIR)\test_basic_operations.obj $(BUILD_DIR)\reg_api.obj
TEST_TARGET = $(BIN_DIR)\test_runner.exe

test: directories $(TEST_TARGET)
	@echo Running tests...
	@$(TEST_TARGET)

$(BUILD_DIR)\test_basic_operations.obj: tests/test_basic_operations.cpp reg_api.h
	$(CC) $(CFLAGS) /c tests/test_basic_operations.cpp /Fo$(BUILD_DIR)\test_basic_operations.obj

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) $(LIBS) /Fe$(TEST_TARGET)
```

### Step 4: Run Tests

```bash
make test
```

---

## 2. Add Error Reporting (20 minutes)

### Step 1: Update Header File

Add to `reg_api.h` in the private section:
```cpp
private:
    mutable LONG m_last_error;
```

Add to public section:
```cpp
public:
    // Get last Windows error code
    LONG get_last_error() const { return m_last_error; }
    
    // Get human-readable error message
    std::string get_last_error_message() const;
```

### Step 2: Update Implementation

Add to `reg_api.cpp`:

```cpp
std::string reg_api::get_last_error_message() const {
    if (m_last_error == ERROR_SUCCESS) {
        return "No error";
    }
    
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        m_last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL
    );
    
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    
    return message;
}
```

Initialize in constructor:
```cpp
reg_api::reg_api(HKEY h_root_key)
    : m_h_root_key(h_root_key)
    , m_h_key(NULL)
    , m_is_open(false)
    , m_last_error(ERROR_SUCCESS)  // Add this
{
}
```

Update `chroot` to store error:
```cpp
bool reg_api::chroot(const std::string& key_path)
{
    close();
    
    LONG result = RegOpenKeyExA(
        m_h_root_key,
        key_path.c_str(),
        0,
        KEY_READ | KEY_WRITE,
        &m_h_key
    );
    
    if (result != ERROR_SUCCESS)
    {
        result = RegCreateKeyExA(
            m_h_root_key,
            key_path.c_str(),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &m_h_key,
            NULL
        );
    }
    
    m_last_error = result;  // Store error code
    m_is_open = (result == ERROR_SUCCESS);
    return m_is_open;
}
```

### Step 3: Usage Example

Update `main.cpp` to show error handling:
```cpp
reg_api reg;
if (!reg.chroot("Software\\MyApplication")) {
    std::cerr << "Failed to open registry key: " 
              << reg.get_last_error_message() 
              << " (Error code: " << reg.get_last_error() << ")" 
              << std::endl;
    return 1;
}
```

---

## 3. Add Documentation Comments (15 minutes)

Update `reg_api.h` with Doxygen-style comments:

```cpp
/**
 * @file reg_api.h
 * @brief Windows Registry API wrapper with type-safe operations
 * @author Your Name
 * @date 2025
 */

/**
 * @class reg_api
 * @brief RAII-based Windows Registry wrapper providing type-safe read/write operations
 * 
 * This class provides a safe and convenient interface to Windows Registry operations.
 * All operations are type-safe through template functions, and numeric values are
 * stored as formatted strings to avoid Windows Registry type limitations.
 * 
 * Key features:
 * - Automatic registry handle management (RAII)
 * - Type-safe numeric read/write operations
 * - Base64 encoding for binary data
 * - POD object serialization
 * - Pointer storage with process ID validation
 * 
 * Thread Safety: This class is NOT thread-safe. Use external synchronization
 * if accessing from multiple threads.
 * 
 * Example usage:
 * @code
 * reg_api reg;
 * if (reg.chroot("Software\\MyApplication")) {
 *     // Write operations
 *     reg.write_string("app_name", "My App");
 *     reg.write_number("version", 123);
 *     
 *     // Read operations with defaults
 *     std::string name = reg.read_string("app_name", "Default");
 *     int version = reg.read_number<int>("version", 0);
 * }
 * @endcode
 */
class reg_api
{
public:
    /**
     * @brief Constructor
     * @param h_root_key Root registry key (default: HKEY_CURRENT_USER)
     */
    reg_api(HKEY h_root_key = HKEY_CURRENT_USER);
    
    /**
     * @brief Destructor - automatically closes open registry key
     */
    ~reg_api();
    
    /**
     * @brief Open or create a registry key
     * @param key_path Path to the registry key (e.g., "Software\\MyApp")
     * @return true if successful, false otherwise
     * @note If key doesn't exist, it will be created
     * @note Use get_last_error() to get detailed error information on failure
     */
    bool chroot(const std::string& key_path);
    
    /**
     * @brief Close the currently open registry key
     */
    void close();
    
    /**
     * @brief Read a numeric value from registry
     * @tparam T Numeric type (int, long, float, double, etc.)
     * @param value_name Name of the registry value
     * @param default_value Value to return if key doesn't exist
     * @return The value from registry or default_value if not found
     * @note Values are stored as formatted strings internally
     */
    template<typename T>
    T read_number(const std::string& value_name, const T& default_value = T());
    
    /**
     * @brief Write a numeric value to registry
     * @tparam T Numeric type (int, long, float, double, etc.)
     * @param value_name Name of the registry value
     * @param value Value to write
     * @return true if successful, false otherwise
     * @note Floating point values are stored with up to 5 decimal places
     * @note Trailing zeros are trimmed
     */
    template<typename T>
    bool write_number(const std::string& value_name, const T& value);
    
    /**
     * @brief Write a pointer value to registry (for debugging/IPC only)
     * @tparam T Type pointed to
     * @param value_name Name of the registry value
     * @param ptr Pointer to store
     * @return true if successful, false otherwise
     * @warning This should only be used for debugging or inter-thread communication
     *          within the same process instance. Pointer values become invalid
     *          after process restart due to ASLR and memory reuse.
     * @note Stores pointer with process ID for validation
     */
    template<typename T>
    bool write_pointer(const std::string& value_name, T* ptr);
    
    /**
     * @brief Read a pointer value from registry
     * @tparam T Type pointed to
     * @param value_name Name of the registry value
     * @param default_value Default pointer to return if not found or invalid
     * @return Pointer value or default_value
     * @warning Only returns valid pointer if process ID matches current process
     * @see write_pointer for important limitations
     */
    template<typename T>
    T* read_pointer(const std::string& value_name, T* default_value = nullptr);
    
    /**
     * @brief Read a string value from registry
     * @param value_name Name of the registry value
     * @param default_value Value to return if key doesn't exist
     * @return The string value or default_value if not found
     */
    std::string read_string(const std::string& value_name, 
                           const std::string& default_value = "");
    
    /**
     * @brief Write a string value to registry
     * @param value_name Name of the registry value
     * @param value String value to write
     * @return true if successful, false otherwise
     */
    bool write_string(const std::string& value_name, const std::string& value);
    
    /**
     * @brief Check if a registry value exists
     * @param value_name Name of the registry value
     * @return true if value exists, false otherwise
     */
    bool value_exists(const std::string& value_name);
    
    /**
     * @brief Delete a registry value
     * @param value_name Name of the registry value to delete
     * @return true if successful, false otherwise
     */
    bool delete_value(const std::string& value_name);
    
    /**
     * @brief Encode a string to Base64
     * @param in Input string
     * @return Base64-encoded string
     */
    std::string b64(const std::string& in);
    
    /**
     * @brief Decode a Base64 string
     * @param in Base64-encoded string
     * @return Decoded string
     */
    std::string b64_d(const std::string& in);
    
    /**
     * @brief Serialize and write a POD object to registry
     * @tparam T Type of object (must be POD or trivially copyable)
     * @param key Name of the registry value
     * @param t_obj Object to serialize
     * @note Object is serialized to bytes and Base64-encoded
     * @warning Only use with POD types or trivially copyable types
     */
    template <typename T>
    void write_obj(const std::string& key, const T& t_obj);
    
    /**
     * @brief Read and deserialize a POD object from registry
     * @tparam T Type of object to read
     * @param key Name of the registry value
     * @return Deserialized object
     * @throws std::runtime_error if key not found or data corrupted
     */
    template <typename T>
    T read_obj(const std::string& key);
    
    /**
     * @brief Get last Windows error code
     * @return Error code from last operation (ERROR_SUCCESS if no error)
     */
    LONG get_last_error() const { return m_last_error; }
    
    /**
     * @brief Get human-readable error message for last error
     * @return Error message string
     */
    std::string get_last_error_message() const;
```

---

## 4. Add Code Formatting (10 minutes)

Create `.clang-format`:
```yaml
---
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
BreakBeforeBraces: Allman
IndentCaseLabels: false
SpaceAfterTemplateKeyword: false
```

Run formatting:
```bash
clang-format -i reg_api.h reg_api.cpp main.cpp
```

---

## 5. Add GitHub Actions CI (10 minutes)

Create `.github/workflows/ci.yml`:
```yaml
name: CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v1
    
    - name: Setup MSVC
      uses: ilammy/msvc-dev-cmd@v1
    
    - name: Build
      run: make all
    
    - name: Run tests
      run: make test
      
    - name: Run example
      run: make run
```

---

## Summary

By following these 5 quick improvements, you will have:

1. ✅ **Unit Testing** - Catch bugs before they reach production
2. ✅ **Error Reporting** - Debug issues more easily
3. ✅ **Documentation** - Help users understand the API
4. ✅ **Code Formatting** - Maintain consistent style
5. ✅ **CI/CD** - Automate testing on every commit

**Time investment: ~1.5 hours**  
**Impact: Transforms the project from prototype to production-ready**

Next steps: Review the full `DEVELOPMENT_ADVICE.md` for additional improvements you can implement over time.
