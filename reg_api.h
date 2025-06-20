#ifndef REGISTRY_HANDLER_H
#define REGISTRY_HANDLER_H

#include <windows.h>
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cassert>

class reg_api
{
public:
    // Constructor
    reg_api(HKEY h_root_key = HKEY_CURRENT_USER);
    
    // Destructor
    ~reg_api();
    
    // Change root key
    bool chroot(const std::string& key_path);
    
    // Close the opened key
    void close();
    
    // Read numeric value (template function) with default value
    template<typename T>
    T read_number(const std::string& value_name, const T& default_value = T());
    
    // Write numeric value (template function)
    template<typename T>
    bool write_number(const std::string& value_name, const T& value);

    template<typename T>
    bool write_pointer(const std::string& value_name, T* ptr);

    template<typename T>
    T* read_pointer(const std::string& value_name, T* default_value = nullptr);
    
    // Read string value with default value
    std::string read_string(const std::string& value_name, const std::string& default_value = "");
    
    // Write string value
    bool write_string(const std::string& value_name, const std::string& value);
    
    // Check if a value exists
    bool value_exists(const std::string& value_name);
    
    // Delete a value
    bool delete_value(const std::string& value_name);

private:
    HKEY m_h_root_key; // Root key handle
    HKEY m_h_key;      // Current open key handle
    bool m_is_open;    // Flag to indicate if a key is open
    
    // Helper function to convert number to string with max 5 decimal places
    template<typename T>
    std::string number_to_string(const T& value);
    
    // Helper function to convert string to number
    template<typename T>
    T string_to_number(const std::string& str, const T& default_val);

    
};

// Template implementation for number to string conversion
template<typename T>
std::string reg_api::number_to_string(const T& value)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << value;
    std::string str = oss.str();
    
    // Trim trailing zeros after decimal point
    if (str.find('.') != std::string::npos)
    {
        str = str.substr(0, str.find_last_not_of('0') + 1);
        if (str.back() == '.')
        {
            str.pop_back();
        }
    }
    
    return str;
}

// Template implementation for string to number conversion
template<typename T>
T reg_api::string_to_number(const std::string& str, const T& default_val)
{
    T result = default_val;
    std::istringstream iss(str);
    
    iss >> result;
    if (iss.fail())
    {
        return default_val;
    }
    
    return result;
}

// Template implementation for reading numeric value
template<typename T>
T reg_api::read_number(const std::string& value_name, const T& default_value)
{
    // Try to read as string
    std::string str_value = read_string(value_name, "");
    
    if (str_value.empty())
    {
        return default_value;
    }
    
    // Convert string to number
    return string_to_number<T>(str_value, default_value);
}

// Template implementation for writing numeric value
template<typename T>
bool reg_api::write_number(const std::string& value_name, const T& value)
{
    // Convert number to string with proper formatting
    std::string str_value = number_to_string<T>(value);
    
    // Write as string
    return write_string(value_name, str_value);
}

// Write pointer value to registry
template<typename T>
bool reg_api::write_pointer(const std::string& value_name, T* ptr)
{
    if (!m_is_open || ptr == nullptr)
    {
        return false;
    }

    // Get current process ID
    DWORD current_pid = GetCurrentProcessId();

    // Convert pointer address to string
    std::ostringstream oss;
    oss << std::hex << reinterpret_cast<uintptr_t>(ptr) << "_" << std::dec << current_pid;

    // Write as string
    return write_string(value_name, oss.str());
}

// Read pointer value from registry with default value
template<typename T>
T* reg_api::read_pointer(const std::string& value_name, T* default_value)
{
      // Try to read as string
    std::string str_value = read_string(value_name, "");
    
    if (str_value.empty())
    {
        return default_value;
    }
    
    // Parse the pointer value and process ID
    size_t separator_pos = str_value.find('_');
    if (separator_pos == std::string::npos)
    {
        return default_value;
    }
    
    std::string ptr_str = str_value.substr(0, separator_pos);
    std::string pid_str = str_value.substr(separator_pos + 1);
    
    // Convert string to pointer and process ID
    uintptr_t ptr_val = 0;
    DWORD stored_pid = 0;
    
    std::istringstream ptr_iss(ptr_str);
    ptr_iss >> std::hex >> ptr_val;
    
    std::istringstream pid_iss(pid_str);
    pid_iss >> stored_pid;
    
    if (ptr_iss.fail() || pid_iss.fail())
    {
        return default_value;
    }
    
    // Get current process ID for validation
    DWORD current_pid = GetCurrentProcessId();
    
    // Validate process ID
    if (current_pid != stored_pid)
    {
        // Process ID mismatch - pointer is from a different process
        std::cerr << "error, not the same PID, stored PID: " << stored_pid 
                  << ", current PID: " << current_pid << std::endl;
        return default_value;
    }
    
    return reinterpret_cast<T*>(ptr_val);
}

#endif // REGISTRY_HANDLER_H
