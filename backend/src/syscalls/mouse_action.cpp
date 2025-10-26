#include "syscalls/mouse_action.h"
#include "syscalls/mouse_tracking.h"
#include <unistd.h>
#include <sys/syscall.h>
#include "syscalls.h"
#include <iostream>

namespace Syscalls {

int clickMouse(int button) {
    if (button != LEFT_CLICK && button != RIGHT_CLICK) {
        std::cerr << " Botón inválido: " << button << std::endl;
        return -1;
    }
    
    std::string button_name = (button == LEFT_CLICK) ? "izquierdo" : "derecho";
    std::cout << "  Haciendo click " << button_name << std::endl;
    
    // Invocar la syscall personalizada mouse_action
    long result = syscall(SYS_MOUSE_ACTION, button);
    
    if (result == 0) {
        std::cout << " Click ejecutado exitosamente" << std::endl;
        return 0;
    } else {
        std::cerr << " Error al hacer click: " << result << std::endl;
        return -1;
    }
}

int clickAt(int x, int y, int button) {
    std::cout << " Click en (" << x << ", " << y << ")" << std::endl;
    
    // Primero mover el mouse a la posición
    if (moveMouse(x, y) != 0) {
        return -1;
    }
    
    // Pequeña pausa para asegurar que el movimiento se completó
    usleep(10000); // 10ms
    
    // Luego hacer el click
    return clickMouse(button);
}

} // namespace Syscalls