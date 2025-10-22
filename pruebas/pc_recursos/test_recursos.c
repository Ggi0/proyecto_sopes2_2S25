#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define __NR_resources_pc 560  // Número de syscall (ajústalo según tu tabla)

struct system_resources {
    unsigned int cpu_usage_percent;
    unsigned int ram_usage_percent;
    unsigned long total_ram_mb;
    unsigned long used_ram_mb;
    unsigned long free_ram_mb;
};

int main(void) {
    struct system_resources res;
    int ret;

    ret = syscall(__NR_resources_pc, &res);
    if (ret < 0) {
        perror("Error al invocar syscall resources_pc");
        return 1;
    }

    printf("Uso de CPU: %u%%\n", res.cpu_usage_percent);
    printf("Uso de RAM: %u%%\n", res.ram_usage_percent);
    printf("RAM Total: %lu MB\n", res.total_ram_mb);
    printf("RAM Usada: %lu MB\n", res.used_ram_mb);
    printf("RAM Libre: %lu MB\n", res.free_ram_mb);

    return 0;
}
