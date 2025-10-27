#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>
#include <vector>

// Definiciones de números de syscalls personalizadas
#define SYS_SCREEN_LIVE 556
#define SYS_MOUSE_TRACKING 557
#define SYS_MOUSE_ACTION 558
#define SYS_KEYBOARD_CAPTION 559
#define SYS_RESOURCES_PC 560

// Estructura para la captura de pantalla
struct screen_capture_info {
    uint32_t width;              // Ancho de la pantalla
    uint32_t height;             // Alto de la pantalla
    uint32_t bytes_per_pixel;    // Bytes por pixel (4 para BGRA)
    uint64_t buffer_size;        // Tamaño total del buffer
    void* data;                  // Puntero al buffer de datos
};

// Estructura para los recursos del sistema
struct system_resources {
    unsigned int cpu_usage_percent;  // Porcentaje de uso de CPU
    unsigned int ram_usage_percent;  // Porcentaje de uso de RAM
    unsigned long total_ram_mb;      // RAM total en MB
    unsigned long used_ram_mb;       // RAM usada en MB
    unsigned long free_ram_mb;       // RAM libre en MB
};


// ==================== ESTRUCTURAS PARA AUTH ====================

/**
 * @brief Niveles de acceso del usuario
 */
enum class AccessLevel {
    NONE,           // Sin acceso
    VIEW_ONLY,      // Solo visualización (grupo remote_view)
    FULL_CONTROL    // Control completo (grupo remote_control)
};

/**
 * @brief Información del usuario autenticado
 */
struct UserInfo {
    std::string username;                  // Nombre de usuario
    std::vector<std::string> groups;       // Grupos a los que pertenece
    AccessLevel access_level;              // Nivel de acceso calculado
    std::string token;                     // JWT token para sesión
    bool authenticated;                    // Estado de autenticación
    
    UserInfo() : access_level(AccessLevel::NONE), authenticated(false) {}
};



// Constantes de configuración
namespace Config {
    const int SCREEN_WIDTH = 1280;
    const int SCREEN_HEIGHT = 800;
    const int BYTES_PER_PIXEL = 4;  // BGRA
    const int FPS = 1;  // Frames por segundo
    const int WEBSOCKET_PORT = 8080;

    // Nombres de grupos para control de acceso
    const char* const GROUP_VIEW = "remote_view";
    const char* const GROUP_CONTROL = "remote_control";

}

#endif // TYPES_H