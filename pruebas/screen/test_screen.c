/* simple test: ajustar __NR_screen_live al número que hayas añadido en tu tabla de syscalls */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define __NR_screen_live 556   /* <- reemplaza por el número real que asignes */

struct screen_capture_info {
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
    uint64_t buffer_size;
    void *data;
};

int main(void)
{
    struct screen_capture_info info;
    int ret;
    void *buf;

    /* inicializamos la estructura con data apuntando a un buffer que reservamos */
    memset(&info, 0, sizeof(info));
    /* Reservamos buffer grande; el kernel retornará buffer_size real en la estructura */
    buf = malloc(1406ULL * 738ULL * 4ULL); /* reserva tentativa */
    if (!buf) {
        perror("malloc");
        return 1;
    }
    info.data = buf;

    /* Llamada a la syscall (reemplaza __NR_screen_live por el número real) */
    ret = syscall(__NR_screen_live, &info);
    if (ret < 0) {
        fprintf(stderr, "screen_live syscall falló: %d (%s)\n", ret, strerror(-ret));
        free(buf);
        return 1;
    }

    printf("screen_live devolvió: %ux%u, bpp=%u, bytes=%llu\n",
           info.width, info.height, info.bytes_per_pixel,
           (unsigned long long)info.buffer_size);

    /* Guardar a un archivo bruto para analizar en userland */
    {
        char fname[64];
        snprintf(fname, sizeof(fname), "screenshot.raw");
        int fd = open(fname, O_CREAT | O_WRONLY, 0644);
        if (fd >= 0) {
            write(fd, buf, (size_t)info.buffer_size);
            close(fd);
            printf("Escrito %llu bytes a %s\n", (unsigned long long)info.buffer_size, fname);
        } else {
            perror("open output");
        }
    }

    free(buf);
    return 0;
}
