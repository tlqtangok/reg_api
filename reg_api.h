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

        // Function to encode a string to Base64
        std::string b64(const std::string& in);

        // Function to decode a Base64 string
        std::string b64_d(const std::string& in);

        template <typename T>
            void write_obj(const std::string& key, const T& t_obj);

        template <typename T>
            T read_obj(const std::string& key);
 
        

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


        // The Base64 character set
        const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        // Function to check if a character is a valid Base64 character
        inline bool is_base64(unsigned char c)
        {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }


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

 template <typename T>
    void reg_api::write_obj(const std::string& key, const T& t_obj)
    {
        // 1. Serialize the object into a byte-string.
        const char* p = reinterpret_cast<const char*>(&t_obj);
        std::string obj_string(p, sizeof(T));

        // 2. Encode the string to Base64.
        std::string encoded_string = b64(obj_string);

        // 3. Write the encoded string to the registry.
        write_string(key, encoded_string);
    }

    /**
     * @brief Reads data from the registry, decodes it, and returns a new POD object.
     * @tparam T The type of the object to read.
     * @param key The registry key to read from.
     * @return The deserialized object of type T.
     * @throws std::runtime_error if the key is not found or data is corrupted.
     */
    template <typename T>
    T reg_api::read_obj(const std::string& key)
    {
        // 1. Read the encoded string from the registry.
        std::string encoded_string = read_string(key, "");
        if (encoded_string.empty())
        {
            throw std::runtime_error("Key not found in registry: " + key);
        }

        // 2. Decode the string from Base64.
        std::string obj_string = b64_d(encoded_string);

        // 3. Check if the size matches the object's size.
        if (obj_string.size() != sizeof(T))
        {
            throw std::runtime_error("Data size mismatch for key: " + key);
        }

        // 4. Deserialize by creating a new object and copying the bytes into it.
        T t_obj;
        memcpy(&t_obj, obj_string.c_str(), sizeof(T));
        return t_obj;
    }


#endif // REGISTRY_HANDLER_H
