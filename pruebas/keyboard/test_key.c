#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define __NR_keyboard_caption 559  // Ajusta al número real en tu kernel

int main(void) {
    int keycode;
    int ret;

    printf("Ingrese un keycode para simular (ejemplo: 30 = A, 28 = ENTER): ");
    scanf("%d", &keycode);

    ret = syscall(__NR_keyboard_caption, keycode);
    if (ret < 0) {
        perror("Error al invocar syscall keyboard_caption");
        return 1;
    }

    printf("Se simuló la tecla con keycode %d exitosamente.\n", keycode);
    return 0;
}
