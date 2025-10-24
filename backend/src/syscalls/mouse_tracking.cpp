#include "syscalls/mouse_tracking.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>

namespace Syscalls {

int moveMouse(int x, int y) {
    std::cout << "  Moviendo mouse a: (" << x << ", " << y << ")" << std::endl;
    
    // Invocar la syscall personalizada mouse_tracking
    long result = syscall(SYS_MOUSE_TRACKING, x, y);
    
    if (result == 0) {
        std::cout << " Mouse movido exitosamente" << std::endl;
        return 0;
    } else {
        std::cerr << " Error al mover mouse: " << result << std::endl;
        return -1;
    }
}

} // namespace Syscalls