#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define __NR_screen_live 556

struct screen_capture_info {
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
    uint64_t buffer_size;
    void *data;
};

int main(void) {
    struct screen_capture_info info;
    int ret;
    void *buf;

    memset(&info, 0, sizeof(info));

    /* Reservar buffer GRANDE (para resoluciones hasta 1920x1080x4) */
    buf = malloc(1920ULL * 1080ULL * 4ULL);
    if (!buf) {
        perror("malloc");
        return 1;
    }

    info.data = buf;

    /* Llamar syscall */
    ret = syscall(__NR_screen_live, &info);
    
    if (ret < 0) {
        fprintf(stderr, "screen_live falló: ret=%d, errno=%d (%s)\n", 
                ret, errno, strerror(errno));
        free(buf);
        return 1;
    }

    printf("✅ Captura exitosa!\n");
    printf("   Resolución: %ux%u\n", info.width, info.height);
    printf("   BPP: %u\n", info.bytes_per_pixel);
    printf("   Bytes: %llu\n", (unsigned long long)info.buffer_size);

    /* Guardar archivo RAW */
    int fd = open("screenshot.raw", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, buf, (size_t)info.buffer_size);
        close(fd);
        printf("   Guardado: screenshot.raw\n");
    }

    free(buf);
    return 0;
}