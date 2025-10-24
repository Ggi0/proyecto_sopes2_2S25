#include "syscalls/keyboard_caption.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <map>
#include <cctype>

namespace Syscalls {

// Mapa de caracteres a keycodes (basado en tu lista)
static const std::map<char, int> char_to_keycode = {
    // Números
    {'1', 2}, {'2', 3}, {'3', 4}, {'4', 5}, {'5', 6},
    {'6', 7}, {'7', 8}, {'8', 9}, {'9', 10}, {'0', 11},
    
    // Letras minúsculas
    {'a', 30}, {'b', 48}, {'c', 46}, {'d', 32}, {'e', 18},
    {'f', 33}, {'g', 34}, {'h', 35}, {'i', 23}, {'j', 36},
    {'k', 37}, {'l', 38}, {'m', 50}, {'n', 49}, {'o', 24},
    {'p', 25}, {'q', 16}, {'r', 19}, {'s', 31}, {'t', 20},
    {'u', 22}, {'v', 47}, {'w', 17}, {'x', 45}, {'y', 21},
    {'z', 44},
    
    // Letras mayúsculas (mismo keycode, el sistema interpreta mayúscula por contexto)
    {'A', 30}, {'B', 48}, {'C', 46}, {'D', 32}, {'E', 18},
    {'F', 33}, {'G', 34}, {'H', 35}, {'I', 23}, {'J', 36},
    {'K', 37}, {'L', 38}, {'M', 50}, {'N', 49}, {'O', 24},
    {'P', 25}, {'Q', 16}, {'R', 19}, {'S', 31}, {'T', 20},
    {'U', 22}, {'V', 47}, {'W', 17}, {'X', 45}, {'Y', 21},
    {'Z', 44},
    
    // Símbolos y caracteres especiales
    {' ', 57},  // Espacio
    {'-', 12}, {'=', 13}, {'[', 26}, {']', 27},
    {';', 39}, {'\'', 40}, {'`', 41}, {'\\', 43},
    {',', 51}, {'.', 52}, {'/', 53},
    
    // Teclas especiales
    {'\n', 28}, // Enter
    {'\t', 15}, // Tab
    {'\b', 14}, // Backspace
};

int pressKey(int keycode) {
    std::cout << "  Presionando tecla con keycode: " << keycode << std::endl;
    
    // Invocar la syscall personalizada keyboard_caption
    long result = syscall(SYS_KEYBOARD_CAPTION, keycode);
    
    if (result == 0) {
        std::cout << " Tecla presionada exitosamente" << std::endl;
        return 0;
    } else {
        std::cerr << " Error al presionar tecla: " << result << std::endl;
        return -1;
    }
}

int charToKeycode(char c) {
    auto it = char_to_keycode.find(c);
    if (it != char_to_keycode.end()) {
        return it->second;
    }
    
    std::cerr << "  Carácter no mapeado: '" << c << "' (ASCII: " << (int)c << ")" << std::endl;
    return -1;
}

int typeText(const std::string& text) {
    std::cout << "  Escribiendo texto: \"" << text << "\"" << std::endl;
    
    int success_count = 0;
    
    for (char c : text) {
        int keycode = charToKeycode(c);
        if (keycode == -1) {
            continue; // Saltar caracteres no mapeados
        }
        
        if (pressKey(keycode) == 0) {
            success_count++;
        }
        
        // Pequeña pausa entre teclas para simular escritura natural
        usleep(50000); // 50ms
    }
    
    std::cout << " Texto escrito: " << success_count << "/" << text.length() << " caracteres" << std::endl;
    
    return success_count;
}

} // namespace Syscalls