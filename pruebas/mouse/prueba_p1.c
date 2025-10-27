#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define el número de la syscall según tu configuración
#define SYSCALL_MOVE_MOUSE 548

/**
 * Función principal para probar la syscall move_mouse
 * Realiza varios movimientos del mouse para verificar su funcionamiento
 */
int main() {
    long ret;
    
    printf("    PRUEBA DE SYSCALL: move_mouse\n");
    
    printf("Esta prueba moverá el mouse en varios patrones.\n");
    printf("Observe el cursor del mouse durante la ejecución.\n\n");
    
    // Prueba 1: Movimiento simple a la derecha y abajo
    printf("[TEST 1] Movimiento simple (300px, 500px)...\n");
    
    ret = syscall(SYSCALL_MOVE_MOUSE, 360, 800);
    
    if (ret == 0) {
        printf("Movimiento exitoso\n");
    } else {
        printf("Error en movimiento: %ld\n", ret);
    }
}