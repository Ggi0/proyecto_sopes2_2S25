#ifndef RESOURCES_PC_H
#define RESOURCES_PC_H

#include "../types.h"
#include <string>

namespace Syscalls {

/**
 * @brief Obtiene los recursos del sistema (CPU y RAM)
 * 
 * Invoca la syscall personalizada resources_pc (560) para obtener
 * información sobre el uso de CPU y memoria RAM
 * 
 * @param resources Estructura donde se almacenarán los datos
 * @return int 0 si es exitoso, -1 en caso de error
 */
int getSystemResources(system_resources& resources);

/**
 * @brief Obtiene recursos en formato JSON
 * 
 * Obtiene los recursos del sistema y los formatea como JSON
 * para envío por WebSocket
 * 
 * @return std::string JSON con cpu_usage y ram_usage
 */
std::string getResourcesJSON();

} // namespace Syscalls

#endif // RESOURCES_PC_H