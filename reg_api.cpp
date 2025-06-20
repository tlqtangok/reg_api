#include "reg_api.h"

reg_api::reg_api(HKEY h_root_key)
    : m_h_root_key(h_root_key)
    , m_h_key(NULL)
    , m_is_open(false)
{
}

reg_api::~reg_api()
{
    close();
}

bool reg_api::chroot(const std::string& key_path)
{
    // Close any previously opened key
    close();
    
    // Try to open the registry key
    LONG result = RegOpenKeyExA(
        m_h_root_key,
        key_path.c_str(),
        0,
        KEY_READ | KEY_WRITE,
        &m_h_key
    );
    
    // If key doesn't exist, try to create it
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
    
    m_is_open = (result == ERROR_SUCCESS);
    return m_is_open;
}

void reg_api::close()
{
    if (m_is_open && m_h_key != NULL)
    {
        RegCloseKey(m_h_key);
        m_h_key = NULL;
        m_is_open = false;
    }
}

std::string reg_api::read_string(const std::string& value_name, const std::string& default_value)
{
    if (!m_is_open || !value_exists(value_name))
    {
        return default_value;
    }
    
    DWORD type = REG_SZ;
    DWORD data_size = 0;
    
    // First query to get the required size
    LONG result = RegQueryValueExA(
        m_h_key,
        value_name.c_str(),
        NULL,
        &type,
        NULL,
        &data_size
    );
    
    if (result != ERROR_SUCCESS)
    {
        return default_value;
    }
    
    // Allocate buffer of appropriate size
    std::vector<char> buffer(data_size);
    
    // Second query to get the actual data
    result = RegQueryValueExA(
        m_h_key,
        value_name.c_str(),
        NULL,
        &type,
        reinterpret_cast<BYTE*>(buffer.data()),
        &data_size
    );
    
    if (result != ERROR_SUCCESS)
    {
        return default_value;
    }
    
    // Ensure null termination
    if (data_size > 0 && buffer[data_size-1] != '\0')
    {
        buffer.push_back('\0');
    }
    
    return std::string(buffer.data());
}



bool reg_api::write_string(const std::string& value_name, const std::string& value)
{
    if (!m_is_open)
    {
        return false;
    }
    
    LONG result = RegSetValueExA(
        m_h_key,
        value_name.c_str(),
        0,
        REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>(value.length() + 1) // Include null terminator
    );
    
    return (result == ERROR_SUCCESS);
}

bool reg_api::value_exists(const std::string& value_name)
{
    if (!m_is_open)
    {
        return false;
    }
    
    DWORD type = 0;
    DWORD data_size = 0;
    
    LONG result = RegQueryValueExA(
        m_h_key,
        value_name.c_str(),
        NULL,
        &type,
        NULL,
        &data_size
    );
    
    return (result == ERROR_SUCCESS);
}

bool reg_api::delete_value(const std::string& value_name)
{
    if (!m_is_open)
    {
        return false;
    }
    
    LONG result = RegDeleteValueA(
        m_h_key,
        value_name.c_str()
    );
    
    return (result == ERROR_SUCCESS);
}


// Function to encode a string to Base64
std::string reg_api::b64(const std::string& in)
{
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
    {
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4)
    {
        out.push_back('=');
    }
    return out;
}

// Function to decode a Base64 string
std::string reg_api::b64_d(const std::string& in)
{
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
    {
        T[base64_chars[i]] = i;
    }

    int val = 0, valb = -8;
    for (unsigned char c : in)
    {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}


