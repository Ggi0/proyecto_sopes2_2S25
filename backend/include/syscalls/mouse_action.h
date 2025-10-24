#ifndef MOUSE_ACTION_H
#define MOUSE_ACTION_H

namespace Syscalls {

/**
 * @brief Tipos de click del mouse
 */
enum MouseButton {
    LEFT_CLICK = 1,   // Click izquierdo
    RIGHT_CLICK = 2   // Click derecho
};

/**
 * @brief Simula un click del mouse
 * 
 * Invoca la syscall personalizada mouse_action (558) para simular
 * un click izquierdo o derecho en la posición actual del cursor
 * 
 * @param button Tipo de click (1 = izquierdo, 2 = derecho)
 * @return int 0 si es exitoso, -1 en caso de error
 */
int clickMouse(int button);

/**
 * @brief Realiza click completo en una posición específica
 * 
 * Combina movimiento y click: primero mueve el mouse a (x,y)
 * y luego ejecuta el click especificado
 * 
 * @param x Coordenada X donde hacer click
 * @param y Coordenada Y donde hacer click
 * @param button Tipo de click (1 = izquierdo, 2 = derecho)
 * @return int 0 si ambas operaciones son exitosas, -1 en caso de error
 */
int clickAt(int x, int y, int button);

} // namespace Syscalls

#endif // MOUSE_ACTION_H