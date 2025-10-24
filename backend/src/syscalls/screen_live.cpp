#include "syscalls/screen_live.h"
#include "utils/base64.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace Syscalls {

int captureScreen(std::vector<unsigned char>& raw_data, screen_capture_info& info) {
    // Limpiar la estructura
    std::memset(&info, 0, sizeof(info));
    
    // Reservar buffer para resolución máxima esperada
    size_t buffer_size = Config::SCREEN_WIDTH * Config::SCREEN_HEIGHT * Config::BYTES_PER_PIXEL;
    raw_data.resize(buffer_size);
    
    // Configurar el puntero al buffer
    info.data = raw_data.data();
    
    // Invocar la syscall personalizada
    long result = syscall(SYS_SCREEN_LIVE, &info);
    
    if (result < 0) {
        std::cerr << " Error al capturar pantalla: " << result << std::endl;
        return -1;
    }
    
    // Ajustar el tamaño del vector al tamaño real capturado
    raw_data.resize(info.buffer_size);
    
    std::cout << " Captura exitosa: " << info.width << "x" << info.height 
              << " (" << info.buffer_size << " bytes)" << std::endl;
    
    return 0;
}

bool convertToJPEG(const std::vector<unsigned char>& raw_data, 
                   std::vector<unsigned char>& jpeg_data,
                   int width, int height) {
    
    // Guardar archivo RAW temporal
    const char* raw_path = "/tmp/screen.raw";
    const char* jpeg_path = "/tmp/screen.jpeg";
    
    std::ofstream raw_file(raw_path, std::ios::binary);
    if (!raw_file) {
        std::cerr << " Error al crear archivo RAW temporal" << std::endl;
        return false;
    }
    
    raw_file.write(reinterpret_cast<const char*>(raw_data.data()), raw_data.size());
    raw_file.close();
    
    // Construir comando de ImageMagick
    std::string command = "convert -size " + std::to_string(width) + "x" + 
                         std::to_string(height) + " -depth 8 bgra:" + 
                         raw_path + " " + jpeg_path;
    
    int ret = system(command.c_str());
    if (ret != 0) {
        std::cerr << " Error al convertir a JPEG" << std::endl;
        return false;
    }
    
    // Leer el archivo JPEG resultante
    std::ifstream jpeg_file(jpeg_path, std::ios::binary | std::ios::ate);
    if (!jpeg_file) {
        std::cerr << " Error al leer archivo JPEG" << std::endl;
        return false;
    }
    
    size_t file_size = jpeg_file.tellg();
    jpeg_file.seekg(0, std::ios::beg);
    
    jpeg_data.resize(file_size);
    jpeg_file.read(reinterpret_cast<char*>(jpeg_data.data()), file_size);
    jpeg_file.close();
    
    // Limpiar archivos temporales
    std::remove(raw_path);
    std::remove(jpeg_path);
    
    std::cout << " JPEG generado: " << file_size << " bytes" << std::endl;
    
    return true;
}

std::string getScreenshotBase64() {
    // Vector para datos crudos
    std::vector<unsigned char> raw_data;
    screen_capture_info info;
    
    // Capturar la pantalla
    if (captureScreen(raw_data, info) != 0) {
        return "";
    }
    
    // Vector para JPEG
    std::vector<unsigned char> jpeg_data;
    
    // Convertir a JPEG
    if (!convertToJPEG(raw_data, jpeg_data, info.width, info.height)) {
        return "";
    }
    
    // Codificar a Base64
    std::string base64_image = Utils::base64Encode(jpeg_data);
    
    return base64_image;
}

} // namespace Syscalls