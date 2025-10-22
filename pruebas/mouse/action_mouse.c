#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

// Número de tu syscall (ajusta según tu tabla)
#define SYS_mouse_action 548

int main(int argc, char *argv[]) {
    int button;
    
    if (argc != 2) {
        printf("Uso: %s <1=izquierdo | 2=derecho>\n", argv[0]);
        return 1;
    }
    
    button = atoi(argv[1]);
    
    if (button != 1 && button != 2) {
        printf("Error: usa 1 para click izquierdo o 2 para derecho\n");
        return 1;
    }
    
    printf("Haciendo click %s...\n", button == 1 ? "izquierdo" : "derecho");
    
    long result = syscall(SYS_mouse_action, button);
    
    if (result == 0) {
        printf("Click exitoso!\n");
    } else {
        printf("Error al hacer click\n");
    }
    
    return result;
}