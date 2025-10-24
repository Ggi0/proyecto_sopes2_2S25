#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>

namespace Utils {

/**
 * @brief Codifica datos binarios a Base64
 * 
 * @param data Vector con los datos binarios a codificar
 * @return std::string Cadena codificada en Base64
 */
std::string base64Encode(const std::vector<unsigned char>& data);

/**
 * @brief Decodifica una cadena Base64 a binario
 * 
 * @param encoded Cadena en Base64
 * @return std::vector<unsigned char> Datos binarios decodificados
 */
std::vector<unsigned char> base64Decode(const std::string& encoded);

} // namespace Utils

#endif // BASE64_H