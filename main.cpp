#include "reg_api.h"
#include <iostream>

#include <vector>
using namespace std;


#include <iostream>
#include <string>
#include <vector>

// // The Base64 character set
// const std::string base64_chars =
//              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//              "abcdefghijklmnopqrstuvwxyz"
//              "0123456789+/";

// // Function to check if a character is a valid Base64 character
// static inline bool is_base64(unsigned char c)
// {
//   return (isalnum(c) || (c == '+') || (c == '/'));
// }

// // Function to encode a string to Base64
// std::string b64(const std::string& in)
// {
//     std::string out;
//     int val = 0, valb = -6;
//     for (unsigned char c : in)
//     {
//         val = (val << 8) + c;
//         valb += 8;
//         while (valb >= 0)
//         {
//             out.push_back(base64_chars[(val >> valb) & 0x3F]);
//             valb -= 6;
//         }
//     }
//     if (valb > -6)
//     {
//         out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
//     }
//     while (out.size() % 4)
//     {
//         out.push_back('=');
//     }
//     return out;
// }

// // Function to decode a Base64 string
// std::string b64_d(const std::string& in)
// {
//     std::string out;
//     std::vector<int> T(256, -1);
//     for (int i = 0; i < 64; i++)
//     {
//         T[base64_chars[i]] = i;
//     }

//     int val = 0, valb = -8;
//     for (unsigned char c : in)
//     {
//         if (T[c] == -1) break;
//         val = (val << 6) + T[c];
//         valb += 6;
//         if (valb >= 0)
//         {
//             out.push_back(char((val >> valb) & 0xFF));
//             valb -= 8;
//         }
//     }
//     return out;
// }


// main_ 
int main()
{
    try 
    {
        // Create reg handler
        reg_api reg;
        
        // Change root to reg key
        if (reg.chroot("Software\\MyApplication"))
        {

            vector<int> int_vector = {11, 22, 33, 44, 55};

            // Writing values
            reg.write_string("app_name", "My Registry Test App");
            reg.write_number<int>("version", 123);
            reg.write_number<double>("pi_value", 3.14159);
            reg.write_number<float>("price", 99.99f);
          
            reg.write_pointer("p_int_vector", &int_vector); 



            // Reading values with defaults
            std::string app_name = reg.read_string("app_name", "Default App Name");
            int version = reg.read_number<int>("version", 0);
            double pi_value = reg.read_number<double>("pi_value", 3.14);
            float price = reg.read_number<float>("price", 0.0f);

            std::vector<int> *p_int_vector = reg.read_pointer<std::vector<int>>("p_int_vector", nullptr);


            struct xyz {
                int x;
                int y;
                int z;
            };

            xyz id_xyz = {11, 22, 33};


            reg.write_obj("xyz", id_xyz); // Write struct to registry
                                          //
            // Reading struct from registry
            xyz read_xyz = reg.read_obj<xyz>("xyz"); // Default values if not found

            std::cout << "Read xyz: x=" << read_xyz.x << ", y=" << read_xyz.y << ", z=" << read_xyz.z << std::endl;













            
            // Reading non-existent value with default
            std::string non_existent = reg.read_string("non_existent", "Default Value");
            int non_existent_num = reg.read_number<int>("non_existent_num", 42);
            
            // Display the read values
            std::cout << "App Name: " << app_name << std::endl;
            std::cout << "Version: " << version << std::endl;
            std::cout << "Pi: " << pi_value << std::endl;
            std::cout << "Price: " << price << std::endl;
            std::cout << "Non-existent: " << non_existent << std::endl;
            std::cout << "Non-existent num: " << non_existent_num << std::endl;
            std::cout << "Pointer to int vector: ";
            for(auto e: *p_int_vector)
            {
                std::cout << e << ",";
            }
            std::cout << std::endl;
            
            // Check if a value exists
            if (reg.value_exists("install_date"))
            {
                std::cout << "install_date exists in reg." << std::endl;
            }
            else
            {
                std::cout << "install_date does not exist in reg." << std::endl;
            }
            
            // Close the reg key
            reg.close();
        }
        else
        {
            std::cout << "Failed to open or create reg key." << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
