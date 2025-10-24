#ifndef MOUSE_TRACKING_H
#define MOUSE_TRACKING_H

namespace Syscalls {

/**
 * @brief Mueve el cursor del mouse a una posición absoluta
 * 
 * Invoca la syscall personalizada mouse_tracking (557) para mover
 * el cursor a las coordenadas especificadas en la pantalla
 * 
 * @param x Coordenada X (horizontal) en píxeles
 * @param y Coordenada Y (vertical) en píxeles
 * @return int 0 si es exitoso, -1 en caso de error
 */
int moveMouse(int x, int y);

} // namespace Syscalls

#endif // MOUSE_TRACKING_H