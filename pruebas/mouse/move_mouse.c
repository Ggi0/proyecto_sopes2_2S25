#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_mouse_tracking 557  // Cambia este número según tu syscall

int main(int argc, char *argv[]) {
    int x, y;
    
    if (argc != 3) {
        printf("Uso: %s <x> <y>\n", argv[0]);
        printf("Ejemplo: %s 500 300\n", argv[0]);
        return 1;
    }
    
    x = atoi(argv[1]);
    y = atoi(argv[2]);
    
    printf("Moviendo mouse a: X=%d, Y=%d\n", x, y);
    
    long result = syscall(SYS_mouse_tracking, x, y);
    
    if (result == 0) {
        printf("Mouse movido exitosamente!\n");
    } else {
        perror("Error al mover mouse");
        return 1;
    }
    
    return 0;
}