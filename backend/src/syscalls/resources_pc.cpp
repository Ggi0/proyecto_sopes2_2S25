#include "syscalls/resources_pc.h"
#include "crow/json.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <cstring>

#include "syscalls.h"

namespace Syscalls {

int getSystemResources(system_resources& resources) {
    // Limpiar la estructura
    std::memset(&resources, 0, sizeof(resources));
    
    // Invocar la syscall personalizada resources_pc
    long result = syscall(SYS_RESOURCES_PC, &resources);
    
    if (result < 0) {
        std::cerr << " Error al obtener recursos del sistema: " << result << std::endl;
        return -1;
    }
    
    std::cout << " Recursos del sistema:" << std::endl;
    std::cout << "   CPU: " << resources.cpu_usage_percent << "%" << std::endl;
    std::cout << "   RAM: " << resources.ram_usage_percent << "%" << std::endl;
    std::cout << "   RAM Total: " << resources.total_ram_mb << " MB" << std::endl;
    std::cout << "   RAM Usada: " << resources.used_ram_mb << " MB" << std::endl;
    std::cout << "   RAM Libre: " << resources.free_ram_mb << " MB" << std::endl;
    
    return 0;
}

std::string getResourcesJSON() {
    system_resources resources;
    
    if (getSystemResources(resources) != 0) {
        // Retornar JSON con valores en 0 si hay error
        crow::json::wvalue json_response;
        json_response["type"] = "resources";
        json_response["cpu_usage"] = 0;
        json_response["ram_usage"] = 0;
        json_response["error"] = "Failed to get system resources";
        return json_response.dump();
    }
    
    // Crear JSON solo con CPU y RAM usage (como solicitaste)
    crow::json::wvalue json_response;
    json_response["type"] = "resources";
    json_response["cpu_usage"] = resources.cpu_usage_percent;
    json_response["ram_usage"] = resources.ram_usage_percent;
    json_response["ram_total"] = resources.total_ram_mb;
    json_response["ram_free"] = resources.free_ram_mb ;
    return json_response.dump();
}

} // namespace Syscalls