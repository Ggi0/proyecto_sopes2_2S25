#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sysinfo.h>
#include <linux/cpumask.h>
#include <linux/tick.h>
#include <linux/kernel_stat.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>

/*
 * Estructura para devolver información de recursos del sistema
 * Esta estructura se comparte entre el kernel y el espacio de usuario
 */
struct system_resources {
    unsigned int cpu_usage_percent;    // Uso de CPU en porcentaje (0-100)
    unsigned int ram_usage_percent;    // Uso de RAM en porcentaje (0-100)
    unsigned long total_ram_mb;        // RAM total en MB
    unsigned long used_ram_mb;         // RAM usada en MB
    unsigned long free_ram_mb;         // RAM libre en MB
};

/*
 * Variables estáticas para calcular el uso de CPU
 * Se guardan los valores anteriores para calcular el delta
 */
static unsigned long long prev_idle_time = 0;
static unsigned long long prev_total_time = 0;
static bool first_call = true;

/*
 * get_cpu_usage - Calcula el porcentaje de uso de CPU del sistema
 * 
 * Funcionamiento:
 *   El uso de CPU se calcula comparando el tiempo total de CPU vs tiempo idle
 *   entre dos mediciones consecutivas.
 * 
 * Fórmula:
 *   CPU_Usage = ((total_delta - idle_delta) / total_delta) * 100
 * 
 * Return: Porcentaje de uso de CPU (0-100)
 */
static unsigned int get_cpu_usage(void)
{
    unsigned long long idle_time = 0;
    unsigned long long total_time = 0;
    unsigned long long idle_delta, total_delta;
    unsigned int cpu_percent;
    int cpu;
    
    /*
     * Recorremos todas las CPUs del sistema
     * En sistemas multi-core, sumamos los tiempos de todas las CPUs
     */
    for_each_possible_cpu(cpu) {
        struct kernel_cpustat *kcs = &kcpustat_cpu(cpu);
        
        /*
         * Obtenemos los tiempos acumulados de CPU en "jiffies"
         * Un jiffy es la unidad de tiempo del kernel (típicamente 1/100 o 1/250 segundo)
         * 
         * Tiempos que se consideran como CPU activa (no idle):
         * - user: Tiempo en modo usuario
         * - nice: Tiempo en modo usuario con prioridad nice
         * - system: Tiempo en modo kernel
         * - irq: Tiempo manejando interrupciones hardware
         * - softirq: Tiempo manejando interrupciones software
         * - steal: Tiempo "robado" por hypervisor (en VMs)
         * 
         * Tiempo idle:
         * - idle: Tiempo sin hacer nada
         * - iowait: Tiempo esperando I/O (también se considera idle)
         */
        
        // Sumamos todos los tiempos para obtener el tiempo TOTAL de esta CPU
        total_time += kcs->cpustat[CPUTIME_USER];      // Tiempo en user mode
        total_time += kcs->cpustat[CPUTIME_NICE];      // Tiempo en user mode (nice)
        total_time += kcs->cpustat[CPUTIME_SYSTEM];    // Tiempo en kernel mode
        total_time += kcs->cpustat[CPUTIME_IDLE];      // Tiempo idle
        total_time += kcs->cpustat[CPUTIME_IOWAIT];    // Tiempo esperando I/O
        total_time += kcs->cpustat[CPUTIME_IRQ];       // Tiempo en interrupciones
        total_time += kcs->cpustat[CPUTIME_SOFTIRQ];   // Tiempo en softirqs
        total_time += kcs->cpustat[CPUTIME_STEAL];     // Tiempo robado (VM)
        
        // Sumamos el tiempo IDLE de esta CPU
        idle_time += kcs->cpustat[CPUTIME_IDLE];
        idle_time += kcs->cpustat[CPUTIME_IOWAIT];
    }
    
    /*
     * En la primera llamada, no podemos calcular el porcentaje
     * porque no tenemos valores previos para comparar
     * Guardamos los valores actuales y retornamos 0
     */
    if (first_call) {
        prev_idle_time = idle_time;
        prev_total_time = total_time;
        first_call = false;
        return 0;
    }
    
    /*
     * Calculamos los deltas (diferencias) desde la última medición
     * 
     * Delta = Valor_Actual - Valor_Anterior
     * 
     * Esto nos da cuánto tiempo ha pasado en cada estado
     * desde la última vez que medimos
     */
    idle_delta = idle_time - prev_idle_time;
    total_delta = total_time - prev_total_time;
    
    // Guardar valores actuales para la próxima llamada
    prev_idle_time = idle_time;
    prev_total_time = total_time;
    
    /*
     * Protección contra división por cero
     * Si no ha pasado tiempo, retornamos el último valor calculado
     */
    if (total_delta == 0) {
        return 0;
    }
    
    /*
     * CÁLCULO DEL PORCENTAJE DE USO DE CPU
     * 
     * Fórmula:
     * CPU_Busy_Time = Total_Time - Idle_Time
     * CPU_Usage% = (CPU_Busy_Time / Total_Time) * 100
     * 
     * Simplificado:
     * CPU_Usage% = ((Total_Delta - Idle_Delta) / Total_Delta) * 100
     * 
     * Usamos multiplicación antes de división para mayor precisión
     * (evitamos pérdida de decimales en división entera)
     */
    cpu_percent = (unsigned int)(((total_delta - idle_delta) * 100) / total_delta);
    
    // Asegurar que el resultado esté en rango 0-100
    if (cpu_percent > 100) {
        cpu_percent = 100;
    }
    
    return cpu_percent;
}

/*
 * get_ram_usage - Obtiene información de uso de memoria RAM
 * @resources: Puntero a estructura donde guardar los resultados
 * 
 * Funcionamiento:
 *   Usa la estructura sysinfo del kernel que contiene estadísticas
 *   de memoria del sistema.
 * 
 * Calcula:
 *   - RAM total disponible
 *   - RAM libre (disponible)
 *   - RAM usada (total - libre)
 *   - Porcentaje de uso
 */
static void get_ram_usage(struct system_resources *resources)
{
    struct sysinfo si;
    unsigned long total_pages, free_pages, used_pages;
    
    /*
     * si_meminfo: Función del kernel que llena la estructura sysinfo
     * con información actual de memoria del sistema
     * 
     * La estructura sysinfo contiene:
     * - totalram: Total de páginas de RAM
     * - freeram: Páginas de RAM libres
     * - bufferram: Páginas usadas para buffers
     * - sharedram: Páginas compartidas
     * - mem_unit: Tamaño de cada unidad de memoria
     */
    si_meminfo(&si);
    
    /*
     * Cálculo de páginas de memoria
     * 
     * El kernel maneja memoria en "páginas"
     * Una página típicamente es 4KB (4096 bytes)
     * 
     * totalram: Total de páginas disponibles en el sistema
     * freeram: Páginas completamente libres (sin usar)
     * bufferram: Páginas usadas como cache/buffer (pueden liberarse si se necesita)
     */
    total_pages = si.totalram;
    
    /*
     * Memoria "libre" incluye:
     * - Páginas completamente libres (freeram)
     * - Páginas usadas como buffer/cache (bufferram)
     * 
     * Los buffers son memoria "reciclable" - el kernel puede liberarla
     * instantáneamente si una aplicación la necesita
     */
    free_pages = si.freeram + si.bufferram;
    
    // Memoria usada = Total - Libre
    used_pages = total_pages - free_pages;
    
    /*
     * Conversión de páginas a Megabytes
     * 
     * Fórmula:
     * MB = (Páginas * Tamaño_Página) / (1024 * 1024)
     * 
     * si.mem_unit: Tamaño de cada unidad de memoria (típicamente 1)
     * PAGE_SIZE: Tamaño de una página (típicamente 4096 bytes)
     * 
     * Dividimos entre (1024 * 1024) = 1048576 para convertir bytes a MB
     */
    resources->total_ram_mb = (total_pages * si.mem_unit * PAGE_SIZE) / (1024 * 1024);
    resources->free_ram_mb = (free_pages * si.mem_unit * PAGE_SIZE) / (1024 * 1024);
    resources->used_ram_mb = (used_pages * si.mem_unit * PAGE_SIZE) / (1024 * 1024);
    
    /*
     * CÁLCULO DEL PORCENTAJE DE USO DE RAM
     * 
     * Fórmula:
     * RAM_Usage% = (Used_RAM / Total_RAM) * 100
     * 
     * Multiplicamos primero por 100 para mantener precisión
     * en división entera
     */
    if (resources->total_ram_mb > 0) {
        resources->ram_usage_percent = 
            (unsigned int)((resources->used_ram_mb * 100) / resources->total_ram_mb);
    } else {
        resources->ram_usage_percent = 0;
    }
    
    // Asegurar que el porcentaje esté en rango 0-100
    if (resources->ram_usage_percent > 100) {
        resources->ram_usage_percent = 100;
    }
}

/*
 * SYSCALL: resources_pc
 * Propósito: Obtener información de uso de recursos del sistema (CPU y RAM)
 * 
 * Parámetros:
 *   @resources_user: Puntero a estructura en espacio de usuario donde
 *                    se copiarán los resultados
 * 
 * Retorno:
 *   0 en éxito
 *   -EFAULT si no se puede copiar datos al espacio de usuario
 *   -EINVAL si el puntero es NULL
 * 
 * Uso desde espacio de usuario:
 *   struct system_resources res;
 *   syscall(560, &res);
 *   printf("CPU: %u%%\n", res.cpu_usage_percent);
 *   printf("RAM: %u%%\n", res.ram_usage_percent);
 */
SYSCALL_DEFINE1(resources_pc, struct system_resources __user *, resources_user)
{
    struct system_resources resources_kernel;
    
    /*
     * Validar que el puntero del usuario no sea NULL
     * __user es una anotación para indicar que el puntero viene
     * del espacio de usuario (ayuda a herramientas de análisis estático)
     */
    if (!resources_user) {
        printk(KERN_ERR "resources_pc: Puntero NULL recibido\n");
        return -EINVAL;
    }
    
    /*
     * Inicializar estructura con ceros
     * Esto asegura que no haya basura en campos no usados
     */
    memset(&resources_kernel, 0, sizeof(struct system_resources));
    
    /*
     * OBTENER USO DE CPU
     * Esta función calcula el porcentaje de uso basado en
     * la diferencia de tiempos desde la última llamada
     */
    resources_kernel.cpu_usage_percent = get_cpu_usage();
    
    /*
     * OBTENER USO DE RAM
     * Esta función llena la estructura con información de memoria:
     * - Porcentaje de uso
     * - Total, usado, libre en MB
     */
    get_ram_usage(&resources_kernel);
    
    /*
     * COPIAR RESULTADOS AL ESPACIO DE USUARIO
     * 
     * copy_to_user: Función segura del kernel para copiar datos
     *               del espacio de kernel al espacio de usuario
     * 
     * Parámetros:
     *   1. Destino en espacio de usuario
     *   2. Origen en espacio de kernel
     *   3. Tamaño a copiar
     * 
     * Retorna: 0 si éxito, número de bytes no copiados si falla
     * 
     * IMPORTANTE: NUNCA usar memcpy para copiar entre espacios
     *             Siempre usar copy_to_user / copy_from_user
     */
    if (copy_to_user(resources_user, &resources_kernel, 
                     sizeof(struct system_resources))) {
        printk(KERN_ERR "resources_pc: Error copiando datos al espacio de usuario\n");
        return -EFAULT;
    }
    
    /*
     * Log de información para debugging
     * Útil para verificar que la syscall está funcionando correctamente
     */
    printk(KERN_INFO "resources_pc: CPU=%u%%, RAM=%u%% (%lu/%lu MB)\n",
           resources_kernel.cpu_usage_percent,
           resources_kernel.ram_usage_percent,
           resources_kernel.used_ram_mb,
           resources_kernel.total_ram_mb);
    
    return 0; // Éxito
}