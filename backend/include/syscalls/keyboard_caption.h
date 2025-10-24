#ifndef KEYBOARD_CAPTION_H
#define KEYBOARD_CAPTION_H

#include <string>

namespace Syscalls {

/**
 * @brief Simula la presión de una tecla
 * 
 * Invoca la syscall personalizada keyboard_caption (559) para simular
 * que se presionó una tecla específica identificada por su keycode
 * 
 * @param keycode Código de la tecla a presionar (ejemplo: 30 = A)
 * @return int 0 si es exitoso, -1 en caso de error
 */
int pressKey(int keycode);

/**
 * @brief Convierte un carácter a su keycode correspondiente
 * 
 * Mapea caracteres comunes a sus keycodes del sistema Linux
 * 
 * @param c Carácter a convertir
 * @return int Keycode correspondiente, -1 si no se encuentra
 */
int charToKeycode(char c);

/**
 * @brief Simula escribir una cadena de texto
 * 
 * Convierte cada carácter de la cadena a su keycode y simula
 * la presión de cada tecla secuencialmente
 * 
 * @param text Texto a escribir
 * @return int Número de teclas presionadas exitosamente
 */
int typeText(const std::string& text);

} // namespace Syscalls

#endif // KEYBOARD_CAPTION_H