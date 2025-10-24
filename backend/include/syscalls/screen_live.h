#ifndef SCREEN_LIVE_H
#define SCREEN_LIVE_H

#include "../types.h"
#include <vector>
#include <string>

namespace Syscalls {

/**
 * @brief Captura la pantalla actual del sistema
 * 
 * Esta función invoca la syscall personalizada screen_live (556) para obtener
 * el contenido actual del framebuffer de la pantalla.
 * 
 * @param raw_data Vector donde se almacenarán los datos crudos BGRA
 * @param info Estructura con información de la captura (ancho, alto, etc.)
 * @return int 0 si es exitoso, -1 en caso de error
 */
int captureScreen(std::vector<unsigned char>& raw_data, screen_capture_info& info);

/**
 * @brief Convierte datos RAW BGRA a formato JPEG
 * 
 * Utiliza ImageMagick para convertir el buffer crudo a JPEG comprimido
 * 
 * @param raw_data Datos crudos BGRA de la captura
 * @param jpeg_data Vector donde se almacenará el JPEG resultante
 * @param width Ancho de la imagen
 * @param height Alto de la imagen
 * @return bool true si la conversión fue exitosa, false en caso contrario
 */
bool convertToJPEG(const std::vector<unsigned char>& raw_data, 
                   std::vector<unsigned char>& jpeg_data,
                   int width, int height);

/**
 * @brief Obtiene un screenshot completo en formato Base64
 * 
 * Función de alto nivel que captura la pantalla, convierte a JPEG
 * y codifica en Base64 para transmisión por WebSocket
 * 
 * @return std::string Imagen en Base64, cadena vacía si hay error
 */
std::string getScreenshotBase64();

} // namespace Syscalls

#endif // SCREEN_LIVE_H