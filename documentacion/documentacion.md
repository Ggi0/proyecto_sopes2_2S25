# Documentacion General

* Giovanni Saul Concoha Cax - 202100229

---


## Detalle de los Pasos Seguidos para Modificar el Kernel, Implementar la Seguridad y Desarrollar la API

### Modificación del Kernel Linux 6.12.41

En esta sección se describe el proceso de personalización del kernel Linux versión 6.12.41. Se explica cómo se descargó el código fuente del kernel, cómo se configuró el entorno de compilación y los ajustes realizados al archivo Makefile para establecer la versión personalizada con EXTRAVERSION. Se detallan los pasos previos necesarios antes de implementar las llamadas al sistema personalizadas.

### 1. Obtención del Código Fuente del Kernel

El primer paso es descargar el código fuente del kernel Linux desde el sitio oficial kernel.org. Se debe utilizar la versión **longterm** más reciente disponible para garantizar estabilidad y soporte a largo plazo.

**Proceso de descarga:**
1. Acceder a https://kernel.org
2. Seleccionar la versión longterm más reciente (en este caso: 6.12.41)
3. Descargar el archivo comprimido

**Extracción del código fuente:**
```bash
tar -xvf linux-6.12.40.tar.xz
```

**Explicación del comando:**
- `tar`: Herramienta para manipular archivos comprimidos
- `-x`: Extraer archivos del archivo
- `-v`: Modo verbose (mostrar archivos siendo extraídos)
- `-f`: Especifica el archivo a procesar

Este comando descomprime aproximadamente 1.2GB de código fuente que incluye:
- Código del kernel principal
- Controladores de dispositivos
- Módulos del sistema
- Archivos de configuración
- Scripts de compilación

### 2. Instalación de Dependencias Necesarias

La compilación del kernel requiere múltiples herramientas y bibliotecas de desarrollo. Cada dependencia cumple un propósito específico:

```bash
sudo apt -y install build-essential libncurses-dev bison flex libssl-dev libelf-dev fakeroot
```

**Descripción detallada de cada dependencia:**

- **build-essential**: Meta-paquete que incluye compiladores esenciales (gcc, g++, make) y herramientas básicas de desarrollo
- **libncurses-dev**: Biblioteca para interfaces de texto enriquecido, necesaria para el menú de configuración del kernel (`make menuconfig`)
- **bison**: Generador de analizadores sintácticos, requerido para procesar archivos de gramática del kernel
- **flex**: Generador de analizadores léxicos, complementa a bison para el análisis de código
- **libssl-dev**: Bibliotecas de desarrollo SSL/TLS, necesarias para funciones criptográficas del kernel
- **libelf-dev**: Bibliotecas para manipular archivos ELF (Executable and Linkable Format), esenciales para módulos del kernel
- **fakeroot**: Permite ejecutar comandos como si fuera root sin permisos reales de superusuario

### 3. Configuración del Kernel

La configuración del kernel es un proceso crítico que determina qué funcionalidades, controladores y módulos se incluirán en la compilación final.

#### 3.1 Generación de Configuración Base

```bash
make localmodconfig
```

**Funcionamiento de localmodconfig:**
Este comando analiza los módulos actualmente cargados en el sistema y genera un archivo `.config` optimizado que incluye únicamente los componentes necesarios para el hardware presente. Esto resulta en:
- Tiempo de compilación reducido
- Kernel más pequeño y eficiente
- Menor superficie de ataque
- Compatibilidad garantizada con el hardware actual

**Proceso interactivo:**
Durante la ejecución, el sistema puede presentar opciones para módulos adicionales no presentes en la configuración actual. La recomendación es responder "N" (No) para mantener una configuración minimalista y estable.

#### 3.2 Desactivación de Claves de Confianza

```bash
scripts/config --disable SYSTEM_TRUSTED_KEYS && \
scripts/config --disable SYSTEM_REVOCATION_KEYS && \
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS "" && \
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```

**Propósito de esta configuración:**
El kernel moderno incluye un sistema de firma de módulos para garantizar la integridad y autenticidad. Sin embargo, durante el desarrollo y testing, estas verificaciones pueden causar problemas:
- Evita errores de compilación relacionados con certificados faltantes
- Permite la carga de módulos sin firma durante el desarrollo
- Simplifica el proceso de testing de nuevas funcionalidades

**Desglose de comandos:**
- `--disable SYSTEM_TRUSTED_KEYS`: Desactiva el uso de claves de confianza del sistema
- `--disable SYSTEM_REVOCATION_KEYS`: Desactiva las claves de revocación
- `--set-str CONFIG_*_KEYS ""`: Establece las rutas de certificados como cadenas vacías

#### 3.3 Personalización del Nombre del Kernel

Para identificar claramente esta versión personalizada del kernel, se debe modificar la variable `EXTRAVERSION` en el Makefile principal:

```bash
# Editar el archivo Makefile en la raíz del código fuente
# Modificar la línea EXTRAVERSION para incluir identificación personal
EXTRAVERSION = -giovanni-concoha-202100229-v4
```



### Implementación de las Llamadas al Sistema Personalizadas

Aquí se documenta la creación de las syscalls personalizadas necesarias para el proyecto. Se explica la ubicación de los archivos modificados o creados dentro de la estructura del kernel, específicamente en los directorios correspondientes. Se detalla el proceso de agregar las nuevas llamadas al sistema en la tabla de syscalls del kernel, los cambios en los archivos de cabecera necesarios, y la implementación del código de cada syscall. Se incluye información sobre las funcionalidades específicas implementadas: monitoreo de CPU, monitoreo de RAM, captura de escritorio, control de teclado y control de cursor.

![estructura kernel](images/1.png)


agregar todas las nuevas llamadas a la tabla de syscalls `syscall_64.tbl`:
![tabla syscalls 64](images/tabla.png)

explicando cada syscall y su funcionamientdo:


#### screen_live.c
### Propósito
Capturar el contenido actual del framebuffer (pantalla) y transferirlo al espacio de usuario para transmisión remota o procesamiento.

### Estructura de Captura

```c
struct screen_capture_info {
    __u32 width;              // Ancho de la pantalla
    __u32 height;             // Alto de la pantalla
    __u32 bytes_per_pixel;    // Bytes por pixel (típicamente 3 o 4)
    __u64 buffer_size;        // Tamaño total del buffer
    void __user *data;        // Puntero al buffer en espacio de usuario
};
```

### Acceso al Framebuffer

```c
extern struct fb_info *registered_fb[];
extern int num_registered_fb;

SYSCALL_DEFINE1(screen_live, struct screen_capture_info __user *, capture_info_user)
{
    struct fb_info *fb_info_ptr;
    
    // Verificar que existe un framebuffer
    if (num_registered_fb < 1 || !registered_fb[0]) {
        return -ENODEV;
    }
    
    fb_info_ptr = registered_fb[0];
}
```

**Framebuffer Linux:** Es la abstracción del kernel que representa la memoria de video. `registered_fb[0]` contiene información del framebuffer primario.

### Sincronización del Framebuffer

```c
if (fb_info_ptr->fbops) {
    // Intentar sincronización
    if (fb_info_ptr->fbops->fb_sync)
        fb_info_ptr->fbops->fb_sync(fb_info_ptr);
    
    // Forzar actualización
    if (fb_info_ptr->fbops->fb_pan_display) {
        struct fb_var_screeninfo var = fb_info_ptr->var;
        fb_info_ptr->fbops->fb_pan_display(&var, fb_info_ptr);
    }
}
```

**Propósito:** Asegurar que el contenido del framebuffer refleje el estado actual de la pantalla antes de capturar.

### Obtención de Metadatos

```c
// Dimensiones de la pantalla
info_k.width = fb_info_ptr->var.xres;
info_k.height = fb_info_ptr->var.yres;
info_k.bytes_per_pixel = fb_info_ptr->var.bits_per_pixel / 8;

// Tamaño total del buffer
fb_size = fb_info_ptr->fix.smem_len;
if (fb_size == 0)
    fb_size = info_k.width * info_k.height * info_k.bytes_per_pixel;
```

### Copia de Datos del Framebuffer

```c
// Alocar buffer temporal en kernel
kbuf = vmalloc(fb_size);

// Copiar desde memoria del framebuffer
if (fb_info_ptr->screen_base) {
    memcpy_fromio(kbuf, fb_info_ptr->screen_base, fb_size);
} else if (fb_info_ptr->screen_buffer) {
    memcpy(kbuf, fb_info_ptr->screen_buffer, fb_size);
}
```

**Diferencia de Funciones:**
- `memcpy_fromio()`: Se usa cuando el framebuffer está mapeado en I/O (memoria de video)
- `memcpy()`: Se usa cuando está en RAM normal

### Transferencia al Espacio de Usuario

```c
// Copiar metadata
if (copy_to_user(capture_info_user, &info_k, sizeof(info_k))) {
    ret = -EFAULT;
    goto cleanup;
}

// Copiar imagen
if (copy_to_user(info_k.data, kbuf, fb_size)) {
    ret = -EFAULT;
    goto cleanup;
}

cleanup:
    vfree(kbuf);  // Liberar buffer temporal
```

**Proceso de Dos Etapas:**
1. Primero se transfiere la metadata (dimensiones, formato)
2. Luego se transfiere el buffer completo de imagen

**Gestión de Memoria:** El uso de `vmalloc()` permite asignar grandes bloques de memoria contiguos virtuales, necesarios para buffers de pantalla que pueden ser de varios MB.





#### mouse_action.c
### Propósito
Simular clicks del mouse (botón izquierdo o derecho), emulando la acción física de presionar y soltar un botón.

### Dispositivo Virtual de Clicks

```c
static struct input_dev *global_click_dev = NULL;
static DEFINE_MUTEX(click_lock);
```

**Diseño Separado:** Se utiliza un dispositivo diferente al de tracking para evitar interferencias entre movimiento y clicks.

### Configuración del Dispositivo

```c
static int init_click_device(void)
{
    global_click_dev = input_allocate_device();
    
    // Configuración
    global_click_dev->name = "Syscall Virtual Mouse Clicks";
    global_click_dev->id.bustype = BUS_USB;
    
    // Habilitar eventos de botones
    set_bit(EV_KEY, global_click_dev->evbit);
    set_bit(BTN_LEFT, global_click_dev->keybit);
    set_bit(BTN_RIGHT, global_click_dev->keybit);
    set_bit(BTN_MIDDLE, global_click_dev->keybit);
    
    // Habilitar movimiento relativo (para reconocimiento como mouse)
    set_bit(EV_REL, global_click_dev->evbit);
    set_bit(REL_X, global_click_dev->relbit);
    set_bit(REL_Y, global_click_dev->relbit);
}
```

**Nota de Compatibilidad:** Aunque no se usa movimiento relativo, se habilita para que el sistema reconozca el dispositivo como un mouse completo.

### Ejecución del Click

```c
SYSCALL_DEFINE1(mouse_action, int, button)
{
    int button_code;
    
    // Validar y traducir botón
    if (button == 1) {
        button_code = BTN_LEFT;
    } else if (button == 2) {
        button_code = BTN_RIGHT;
    } else {
        return -EINVAL;
    }
    
    mutex_lock(&click_lock);
    
    // Presionar botón
    input_report_key(global_click_dev, button_code, 1);
    input_sync(global_click_dev);
    
    // Pausa de 50ms
    msleep(50);
    
    // Soltar botón
    input_report_key(global_click_dev, button_code, 0);
    input_sync(global_click_dev);
    
    mutex_unlock(&click_lock);
}
```

**Protocolo de Click:**
1. Traducción del parámetro numérico a constante del kernel (BTN_LEFT/BTN_RIGHT)
2. Presión del botón con valor `1`
3. Pausa de 50ms (simula duración física del click)
4. Liberación del botón con valor `0`



#### mouse_tracking.c
### Propósito
Mover el cursor del mouse a coordenadas absolutas específicas en la pantalla, permitiendo el control remoto del puntero.

### Características del Dispositivo Virtual

```c
static struct input_dev *global_mouse_dev = NULL;
static DEFINE_MUTEX(mouse_lock);
```

**Diferencia Clave:** A diferencia del teclado, este dispositivo usa coordenadas **absolutas** en lugar de movimiento relativo.

### Configuración del Dispositivo

```c
static int init_mouse_device(void)
{
    global_mouse_dev = input_allocate_device();
    
    // Configuración de identificación
    global_mouse_dev->name = "Syscall Virtual Mouse";
    global_mouse_dev->id.bustype = BUS_USB;  // BUS_USB para mejor compatibilidad
    
    // Habilitar movimiento ABSOLUTO
    set_bit(EV_ABS, global_mouse_dev->evbit);
    set_bit(ABS_X, global_mouse_dev->absbit);
    set_bit(ABS_Y, global_mouse_dev->absbit);
    
    // Habilitar clicks (compatibilidad)
    set_bit(EV_KEY, global_mouse_dev->evbit);
    set_bit(BTN_LEFT, global_mouse_dev->keybit);
    set_bit(BTN_RIGHT, global_mouse_dev->keybit);
    
    // Configurar rangos para resolución 1280x800
    input_set_abs_params(global_mouse_dev, ABS_X, 0, 1279, 0, 0);
    input_set_abs_params(global_mouse_dev, ABS_Y, 0, 799, 0, 0);
}
```

**Parámetros de `input_set_abs_params()`:**
- Dispositivo
- Eje (X o Y)
- Valor mínimo (0)
- Valor máximo (1279 para X, 799 para Y)
- Fuzz (0 - sin suavizado)
- Flat (0 - sin zona muerta)

### Movimiento del Cursor

```c
SYSCALL_DEFINE2(mouse_tracking, int, x, int, y)
{
    // Validar coordenadas para resolución 1280x800
    if (x < 0 || x >= 1280 || y < 0 || y >= 800) {
        return -EINVAL;
    }
    
    mutex_lock(&mouse_lock);
    
    // Enviar evento de movimiento absoluto
    input_report_abs(global_mouse_dev, ABS_X, x);
    input_report_abs(global_mouse_dev, ABS_Y, y);
    input_sync(global_mouse_dev);
    
    mutex_unlock(&mouse_lock);
}
```

**Aspectos Importantes:**
- `input_report_abs()`: Reporta posición absoluta (no movimiento relativo)
- Las coordenadas son validadas contra la resolución configurada
- Un solo `input_sync()` es suficiente para ambas coordenadas
- El sistema operativo interpreta estos eventos como movimiento instantáneo del cursor



#### resources_pc.c
### Propósito
Obtener información en tiempo real sobre el uso de recursos del sistema, específicamente CPU y RAM.

### Estructura de Datos

```c
struct system_resources {
    unsigned int cpu_usage_percent;    // Uso de CPU en porcentaje (0-100)
    unsigned int ram_usage_percent;    // Uso de RAM en porcentaje (0-100)
    unsigned long total_ram_mb;        // RAM total en MB
    unsigned long used_ram_mb;         // RAM usada en MB
    unsigned long free_ram_mb;         // RAM libre en MB
};
```

### Funcionamiento Interno

#### Cálculo del Uso de CPU

La función `get_cpu_usage()` implementa un algoritmo de medición diferencial que calcula el porcentaje de uso de CPU comparando dos mediciones consecutivas:

```c
static unsigned int get_cpu_usage(void)
{
    unsigned long long idle_time = 0;
    unsigned long long total_time = 0;
    
    // Recorrer todas las CPUs del sistema
    for_each_possible_cpu(cpu) {
        struct kernel_cpustat *kcs = &kcpustat_cpu(cpu);
        
        // Acumular tiempos de CPU activa
        total_time += kcs->cpustat[CPUTIME_USER];      
        total_time += kcs->cpustat[CPUTIME_SYSTEM];    
        total_time += kcs->cpustat[CPUTIME_IDLE];      
        // ... otros estados
        
        // Acumular tiempo idle
        idle_time += kcs->cpustat[CPUTIME_IDLE];
        idle_time += kcs->cpustat[CPUTIME_IOWAIT];
    }
}
```

**Fórmula de Cálculo:**
```
CPU_Usage% = ((Total_Delta - Idle_Delta) / Total_Delta) * 100
```

Donde:
- `Total_Delta`: Diferencia del tiempo total de CPU desde la última medición
- `Idle_Delta`: Diferencia del tiempo idle desde la última medición

**Aspectos Clave:**
- Utiliza variables estáticas para mantener estado entre llamadas
- En la primera llamada retorna 0 porque no hay valores previos para comparar
- Los tiempos se miden en "jiffies" (unidad de tiempo del kernel)

#### Cálculo del Uso de RAM

La función `get_ram_usage()` obtiene estadísticas de memoria del sistema:

```c
static void get_ram_usage(struct system_resources *resources)
{
    struct sysinfo si;
    
    // Obtener información del sistema
    si_meminfo(&si);
    
    // Calcular páginas de memoria
    total_pages = si.totalram;
    free_pages = si.freeram + si.bufferram;
    used_pages = total_pages - free_pages;
    
    // Convertir páginas a MB
    resources->total_ram_mb = (total_pages * si.mem_unit * PAGE_SIZE) / (1024 * 1024);
}
```

**Aspectos Importantes:**
- La memoria se maneja en páginas (típicamente 4KB)
- Los buffers se consideran memoria "reciclable" y se suman a la memoria libre
- La conversión incluye el factor `si.mem_unit` que normaliza las unidades

### Transferencia de Datos al Espacio de Usuario

```c
if (copy_to_user(resources_user, &resources_kernel, 
                 sizeof(struct system_resources))) {
    return -EFAULT;
}
```

**Punto Crítico:** Nunca se debe usar `memcpy()` para copiar datos entre el espacio de kernel y el espacio de usuario. Siempre debe utilizarse `copy_to_user()` que realiza las validaciones de seguridad necesarias.



#### keyboard_caption.c
### Propósito
Simular la pulsación de teclas en el sistema operativo, emulando eventos de teclado como si fueran generados por un dispositivo físico.

### Arquitectura del Dispositivo Virtual

Esta syscall crea y mantiene un dispositivo de entrada virtual persistente que permanece activo entre llamadas:

```c
static struct input_dev *global_virtual_kbd = NULL;
static DEFINE_MUTEX(kbd_caption_mutex);
static bool kbd_caption_initialized = false;
```

**Patrón de Diseño:** Singleton con inicialización perezosa (lazy initialization)

### Inicialización del Dispositivo Virtual

```c
static int init_virtual_keyboard_caption(void)
{
    // Crear dispositivo de entrada virtual
    global_virtual_kbd = input_allocate_device();
    
    // Configurar propiedades del dispositivo
    global_virtual_kbd->name = "Virtual Keyboard Caption";
    global_virtual_kbd->phys = "syscall/kbd_caption0";
    global_virtual_kbd->id.bustype = BUS_VIRTUAL;
    
    // Habilitar tipos de eventos
    __set_bit(EV_KEY, global_virtual_kbd->evbit);
    __set_bit(EV_SYN, global_virtual_kbd->evbit);
    
    // Habilitar todas las teclas posibles
    for (i = 0; i <= KEY_MAX; i++) {
        __set_bit(i, global_virtual_kbd->keybit);
    }
    
    // Registrar en el sistema
    err = input_register_device(global_virtual_kbd);
}
```

**Aspectos Técnicos:**
- `BUS_VIRTUAL`: Indica que no es un dispositivo físico
- `EV_KEY`: Habilita eventos de teclado
- `EV_SYN`: Habilita eventos de sincronización
- El dispositivo se registra en `/dev/input/` y es visible para el sistema

### Simulación de Pulsación de Tecla

El proceso de simulación consta de tres fases:

```c
SYSCALL_DEFINE1(keyboard_caption, int, keycode)
{
    mutex_lock(&kbd_caption_mutex);
    
    // FASE 1: PRESIONAR LA TECLA (keydown)
    input_report_key(global_virtual_kbd, keycode, 1);
    input_sync(global_virtual_kbd);
    
    // PAUSA ENTRE KEYDOWN Y KEYUP
    msleep(50);
    
    // FASE 2: SOLTAR LA TECLA (keyup)
    input_report_key(global_virtual_kbd, keycode, 0);
    input_sync(global_virtual_kbd);
    
    mutex_unlock(&kbd_caption_mutex);
}
```

**Flujo de Eventos:**
1. **Keydown**: Se reporta la tecla con valor `1` (presionada)
2. **Sincronización**: `input_sync()` notifica al sistema que el evento está completo
3. **Pausa**: `msleep(50)` simula el tiempo físico de pulsación
4. **Keyup**: Se reporta la tecla con valor `0` (liberada)
5. **Sincronización**: Nueva sincronización para completar el ciclo

**Protección de Concurrencia:** El mutex `kbd_caption_mutex` garantiza que múltiples llamadas simultáneas no interfieran entre sí.




### Compilación e Instalación del Kernel Modificado

Esta sección describe el proceso completo de compilación del kernel modificado. Se documentan los comandos utilizados para configurar el kernel, el proceso de compilación, la instalación de los módulos y la actualización del gestor de arranque GRUB. Se incluyen los pasos para verificar que el sistema arranque correctamente con el nuevo kernel y cómo validar que la versión personalizada se haya instalado exitosamente.

### 4. Compilación del Kernel

La compilación es el proceso más intensivo en recursos de toda la práctica.

#### 4.1 Comando de Compilación Optimizado

```bash
fakeroot make -j$(nproc) 2>&1 | tee build.log
```

**Análisis detallado del comando:**

**fakeroot**: Ejecuta la compilación simulando permisos de root, necesario para:
- Crear archivos con permisos específicos del sistema
- Evitar problemas de permisos durante la instalación
- Mantener la seguridad del sistema de desarrollo

**make**: El sistema de construcción del kernel que:
- Procesa el Makefile principal y submakefiles
- Coordina la compilación de miles de archivos fuente
- Maneja dependencias entre componentes
- Genera el kernel final y módulos

**-j$(nproc)**: Paralelización óptima donde:
- `$(nproc)` detecta automáticamente el número de núcleos de CPU disponibles
- `-j` especifica el número de trabajos paralelos
- Maximiza el uso de recursos del sistema
- Reduce significativamente el tiempo de compilación

**2>&1**: Redirección que:
- Combina la salida estándar (stdout) y error estándar (stderr)
- Asegura que todos los mensajes se capturen en el log
- Facilita la depuración de errores de compilación

**| tee build.log**: 
- Divide la salida para mostrarla en pantalla y guardarla simultáneamente
- Permite monitorear el progreso en tiempo real
- Genera un log completo para análisis posterior

#### 4.2 Proceso de Compilación

El proceso de compilación típicamente incluye las siguientes fases:

1. **Preparación del entorno**: Verificación de dependencias y configuración
2. **Compilación del kernel principal**: Código core del sistema operativo
3. **Compilación de módulos**: Controladores y funcionalidades opcionales
4. **Vinculación**: Unión de todos los componentes compilados
5. **Generación de imágenes**: Creación de archivos ejecutables finales

**Tiempo de compilación esperado:**
- CPU modernas (8+ núcleos): 30-60 minutos
- CPU de gama media (4-8 núcleos): 1-2 horas
- CPU más antiguas (<4 núcleos): 2+ horas

#### 4.3 Resultado de la Compilación

Al finalizar exitosamente, se habrán generado:
- **vmlinuz**: Imagen comprimida del kernel
- **System.map**: Tabla de símbolos del kernel
- **Módulos compilados**: Archivos .ko en diversas ubicaciones
- **Archivos de configuración**: Para debugging y análisis

### 5. Instalación del Kernel

#### 5.1 Instalación de Módulos

```bash
sudo make modules_install
```

**Función de este comando:**
- Copia todos los módulos compilados (.ko) a `/lib/modules/[versión]/`
- Crea la estructura de directorios necesaria
- Actualiza las bases de datos de dependencias de módulos
- Prepara el sistema para utilizar los nuevos controladores

#### 5.2 Instalación del Kernel Principal

```bash
sudo make install
```

**Acciones realizadas:**
- Copia el kernel compilado a `/boot/`
- Instala System.map para debugging
- Actualiza el bootloader (GRUB)
- Crea entrada de arranque para el nuevo kernel

### 6. Configuración del Bootloader

#### 6.1 Habilitación del Menú de Arranque

La configuración del bootloader GRUB es esencial para poder seleccionar el kernel personalizado al inicio del sistema.

**Edición del archivo de configuración GRUB:**
```bash
sudo nano /etc/default/grub
```

**Configuración optimizada:**
```bash
# If you change this file, run 'update-grub' afterwards to update
# /boot/grub/grub.cfg.
# For full documentation of the options in this file, see:
#   info -f grub -n 'Simple configuration'

GRUB_DEFAULT=saved
GRUB_TIMEOUT_STYLE=menu
GRUB_TIMEOUT=30
GRUB_SAVEDEFAULT=true
GRUB_DISTRIBUTOR=`( . /etc/os-release; echo ${NAME:-Ubuntu} ) 2>/dev/null || echo Ubuntu`
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
GRUB_CMDLINE_LINUX=""
```

**Explicación de parámetros clave:**

- **GRUB_DEFAULT=saved**: Mantiene la última selección del usuario como predeterminada
- **GRUB_TIMEOUT_STYLE=menu**: Fuerza la visualización del menú de arranque
- **GRUB_TIMEOUT=10**: Tiempo de espera de 10 segundos para selección automática
- **GRUB_SAVEDEFAULT=true**: Guarda la selección del usuario para futuros arranques

#### 6.2 Actualización de la Configuración

```bash
sudo update-grub
```

Este comando:
- Regenera el archivo `/boot/grub/grub.cfg`
- Detecta todos los kernels instalados
- Crea entradas de menú apropiadas
- Aplica la nueva configuración

### 7. Verificación del Kernel Personalizado

#### 7.1 Reinicio y Selección

Después del reinicio, el menú GRUB mostrará las opciones disponibles, incluyendo el kernel personalizado.

#### 7.2 Verificación de la Instalación

Una vez iniciado el sistema con el nuevo kernel, se puede verificar con:

```bash
uname -r
```

**Salida esperada:**
```
6.12.41-gio-concoha-202100229-proyecto
```



### Pruebas Individuales de las Llamadas al Sistema

En esta parte se verificó el correcto funcionamiento de cada syscall de manera aislada. Se describe la creación de programas de prueba en C que invocan directamente las llamadas al sistema, sin intermediarios. Se documentan los resultados esperados de cada prueba y cómo se validó que las syscalls respondieran correctamente antes de integrarlas con el resto del sistema.

![archivos prueba C](images/prueba.png)

### Desarrollo del Backend con C++ y Crow

Aquí se detalla la implementación del servidor API utilizando C++ y el framework Crow. Se explica la estructura del proyecto backend, la configuración inicial del framework, y la creación de los endpoints básicos que exponen las funcionalidades de las syscalls. Se describe cómo se estableció la comunicación entre el código del backend y las llamadas al sistema del kernel modificado, incluyendo el manejo de respuestas y el formato de datos utilizado (JSON).

El backend fue desarrollado en C++ utilizando el framework Crow para manejar peticiones HTTP y WebSockets. La estructura del proyecto está organizada de la siguiente manera:

```
backend/
├── CMakeLists.txt              # Configuración de CMake para compilación
├── Crow/                       # Framework Crow (header-only)
├── build/                      # Directorio de compilación (generado)
├── api_docu.md                 # Documentación de la API
├── include/                    # Headers del proyecto
│   ├── types.h                 # Definiciones de tipos y estructuras
│   ├── syscalls.h              # Declaraciones generales de syscalls
│   ├── handlers/               # Manejadores de rutas HTTP y WebSocket
│   │   ├── http_handler.h      # Controladores de endpoints HTTP
│   │   ├── websocket_handler.h # Manejador de conexiones WebSocket
│   │   └── auth_handler.h      # Manejador de autenticación
│   ├── syscalls/               # Wrappers para llamadas al sistema
│   │   ├── resources_pc.h      # Wrapper para syscall de recursos
│   │   ├── screen_live.h       # Wrapper para captura de pantalla
│   │   ├── keyboard_caption.h  # Wrapper para simulación de teclado
│   │   ├── mouse_tracking.h    # Wrapper para movimiento de mouse
│   │   └── mouse_action.h      # Wrapper para clicks de mouse
│   ├── utils/                  # Utilidades auxiliares
│   │   └── base64.h            # Codificación/decodificación Base64
│   └── auth/                   # Módulos de autenticación
└── src/                        # Código fuente (.cpp)
    ├── main.cpp                # Punto de entrada del servidor
    ├── http_handler.cpp        # Implementación de endpoints HTTP
    ├── websocket_handler.cpp   # Implementación de WebSocket
    ├── resources_pc.cpp        # Implementación wrapper recursos
    ├── screen_live.cpp         # Implementación captura de pantalla
    ├── keyboard_caption.cpp    # Implementación simulación teclado
    ├── mouse_tracking.cpp      # Implementación movimiento mouse
    ├── mouse_action.cpp        # Implementación clicks mouse
    └── auth_handler.cpp        # Implementación autenticación
```

## Instalación del Framework Crow

Crow es un framework web minimalista para C++ inspirado en Flask (Python). Es un framework "header-only", lo que significa que solo necesita incluir los archivos de cabecera sin necesidad de compilar una librería separada.

### Instalación de Dependencias

Crow requiere CMake y las librerías Boost para su funcionamiento:

```bash
sudo apt update
sudo apt install cmake libboost-all-dev
```

**Dependencias instaladas:**
- **CMake**: Sistema de construcción multiplataforma
- **Boost**: Colección de librerías C++ para diversas funcionalidades (asio para operaciones asíncronas, entre otras)

### Obtención de Crow

Se clonó el repositorio oficial de Crow y se copió a la carpeta del proyecto:

```bash
git clone https://github.com/CrowCpp/Crow.git
```

El repositorio de Crow contiene el archivo de cabecera único que se incluye en el proyecto. Este archivo se encuentra en la carpeta `Crow/` dentro del directorio backend.

### Integración con el Proyecto

Crow se integra en el código simplemente incluyendo su header principal:

```cpp
#include "crow.h"
```

No requiere linkeo de librerías adicionales ya que todas sus funcionalidades están contenidas en los headers.

## Compilación del Proyecto

El proyecto utiliza CMake como sistema de construcción, lo que permite una compilación organizada y portable.

### Proceso de Compilación

Desde el directorio raíz del backend, ejecutar los siguientes comandos:

```bash
# Crear directorio de compilación
mkdir -p build

# Entrar al directorio de compilación
cd build

# Generar archivos de construcción con CMake
cmake ..

# Compilar el proyecto
make
```

**Explicación del proceso:**

1. **mkdir -p build**: Crea el directorio donde se generarán los archivos compilados, manteniendo limpio el código fuente
2. **cmake ..**: Analiza el archivo CMakeLists.txt y genera los Makefiles necesarios para la compilación
3. **make**: Compila todos los archivos fuente y genera el ejecutable final

### Ejecución del Servidor

Una vez compilado exitosamente, el ejecutable se encuentra en el directorio `build/`:

```bash
# Ejecutar el servidor (desde build/)
./servidor
```

El servidor quedará escuchando en el puerto configurado (típicamente 8080) y estará listo para recibir peticiones HTTP y conexiones WebSocket.

### Recompilación

Para recompilar después de realizar cambios en el código:

```bash
# Desde el directorio build/
make
```

No es necesario volver a ejecutar `cmake ..` a menos que se hayan modificado archivos del sistema de construcción (CMakeLists.txt) o se hayan agregado nuevos archivos fuente.

## Arquitectura del Backend

El backend sigue una arquitectura modular en capas:

### Capa de Presentación (Handlers)
Contiene los manejadores de rutas HTTP y WebSocket que reciben las peticiones de los clientes. Estos handlers validan las peticiones, extraen los parámetros necesarios y delegan el procesamiento a las capas inferiores.

### Capa de Lógica de Negocio (Syscalls Wrappers)
Implementa funciones C++ que actúan como intermediarias entre los handlers y las llamadas al sistema del kernel. Estas funciones encapsulan la complejidad de invocar syscalls, manejan la conversión de datos entre formatos de usuario y kernel, y procesan las respuestas.

### Capa de Sistema (Syscalls)
Invoca directamente las syscalls personalizadas del kernel modificado utilizando la función `syscall()` de la librería estándar de C. Esta es la interfaz directa con el kernel Linux.

### Utilidades
Módulos auxiliares que proporcionan funcionalidades transversales como codificación Base64 para transmisión de imágenes, manejo de errores, y funciones auxiliares para procesamiento de datos.

## Configuración del CMakeLists.txt

El archivo CMakeLists.txt configura todo el proceso de compilación del proyecto:

```cmake
cmake_minimum_required(VERSION 3.10)
project(ServidorRemoto)

# Estándar C++14 requerido por Crow
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluir directorios de headers
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/Crow/include)

# Buscar librerías necesarias
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

# Agregar todos los archivos fuente
file(GLOB SOURCES "src/*.cpp")

# Crear ejecutable
add_executable(servidor ${SOURCES})

# Linkear librerías
target_link_libraries(servidor Threads::Threads ${Boost_LIBRARIES})
```

Este archivo define las dependencias, los directorios de inclusión, y cómo se deben compilar y enlazar todos los componentes del proyecto.






### Pruebas del Backend con Thunder Client

Esta sección documenta el proceso de validación del backend mediante pruebas con Thunder Client (o herramientas similares). Se explican las pruebas realizadas para cada endpoint creado, los payloads de ejemplo utilizados, las respuestas esperadas del servidor y cómo se verificó que la comunicación entre el backend y el kernel funcionara correctamente. Se incluyen capturas o ejemplos de las peticiones HTTP realizadas y sus respuestas.

# Endpoints del Backend y Protocolo WebSocket

## Endpoints HTTP

El backend expone una API RESTful para autenticación y control remoto del sistema. Todos los endpoints (excepto `/health`) requieren autenticación mediante tokens.

### 1. Endpoint de Salud del Servidor

**Ruta:** `GET /health`  
**Autenticación:** No requerida  
**Propósito:** Verificar que el servidor está activo y respondiendo

```bash
curl -X GET http://10.150.1.233:8080/health
```

**Respuesta esperada:**
```json
{
  "status": "ok",
  "message": "Servidor funcionando correctamente",
  "timestamp": 1234567890
}
```

Este endpoint es utilizado para monitoreo básico y verificación de conectividad antes de intentar operaciones más complejas.

---

### 2. Endpoint de Login (Autenticación)

**Ruta:** `POST /api/auth/login`  
**Autenticación:** No requerida (es el endpoint que genera tokens)  
**Propósito:** Autenticar usuario mediante PAM y generar token de sesión

**Petición:**
```bash
curl -X POST http://10.150.1.233:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "admin",
    "password": "123"
  }'
```

**Parámetros del payload:**
- `username`: Nombre de usuario del sistema Linux
- `password`: Contraseña del usuario

**Respuesta exitosa (200 OK):**
```json
{
  "success": true,
  "username": "admin",
  "access_level": "full_control",
  "token": "abc123def456ghi789...",
  "groups": ["admin", "remote_control"],
  "can_view": true,
  "can_control": true
}
```

**Campos de la respuesta:**
- `success`: Indica si la autenticación fue exitosa
- `username`: Usuario autenticado
- `access_level`: Nivel de acceso (`view_only` o `full_control`)
- `token`: Token de sesión para futuras peticiones
- `groups`: Lista de grupos a los que pertenece el usuario
- `can_view`: Permiso para ver el escritorio remoto
- `can_control`: Permiso para controlar el equipo

**Respuesta de error (401 Unauthorized):**
```json
{
  "success": false,
  "error": "Credenciales inválidas"
}
```

**Proceso interno:**
1. El servidor recibe las credenciales
2. Valida contra PAM del sistema Linux
3. Verifica la pertenencia a grupos `remote_view` o `remote_control`
4. Genera un token único de sesión
5. Almacena el token en memoria asociado al usuario
6. Retorna el token y los permisos

---

### 3. Endpoint de Logout (Cierre de Sesión)

**Ruta:** `POST /api/auth/logout`  
**Autenticación:** Requerida (token en header)  
**Propósito:** Invalidar el token de sesión actual

**Petición:**
```bash
TOKEN="abc123def456ghi789..."
curl -X POST http://10.150.1.233:8080/api/auth/logout \
  -H "Authorization: Bearer $TOKEN"
```

**Respuesta exitosa (200 OK):**
```json
{
  "success": true,
  "message": "Sesión cerrada correctamente"
}
```

**Proceso interno:**
1. El servidor extrae el token del header `Authorization`
2. Elimina el token de la tabla de sesiones activas
3. El token queda invalidado y no puede usarse nuevamente

---

### 4. Endpoint de Click del Mouse

**Ruta:** `POST /api/mouse/click`  
**Autenticación:** Requerida + Permiso `full_control`  
**Propósito:** Ejecutar un click de mouse en coordenadas específicas

**Petición:**
```bash
TOKEN="abc123def456ghi789..."
curl -X POST http://10.150.1.233:8080/api/mouse/click \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "x": 500,
    "y": 300,
    "button": 1
  }'
```

**Parámetros del payload:**
- `x`: Coordenada X del click (0-1279)
- `y`: Coordenada Y del click (0-799)
- `button`: Botón del mouse
  - `1`: Botón izquierdo
  - `2`: Botón derecho

**Respuesta exitosa (200 OK):**
```json
{
  "success": true,
  "message": "Click ejecutado correctamente",
  "x": 500,
  "y": 300,
  "button": "left"
}
```

**Respuesta de error - Sin permisos (403 Forbidden):**
```json
{
  "success": false,
  "error": "Acceso denegado: requiere permiso de control completo"
}
```

**Proceso interno:**
1. Valida el token de autenticación
2. Verifica que el usuario tenga permisos de `full_control`
3. Valida las coordenadas dentro del rango permitido
4. Invoca syscall `mouse_tracking` para mover el cursor
5. Invoca syscall `mouse_action` para ejecutar el click
6. Retorna el resultado de la operación

---

### 5. Endpoint de Presión de Tecla

**Ruta:** `POST /api/keyboard/press`  
**Autenticación:** Requerida + Permiso `full_control`  
**Propósito:** Simular la pulsación de una tecla

**Petición por carácter:**
```bash
TOKEN="abc123def456ghi789..."
curl -X POST http://10.150.1.233:8080/api/keyboard/press \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"key": "a"}'
```

**Petición por keycode:**
```bash
curl -X POST http://10.150.1.233:8080/api/keyboard/press \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"keycode": 30}'
```

**Parámetros del payload (dos opciones):**
- Opción 1: `key`: Carácter a presionar (ej: "a", "A", "1", "Enter")
- Opción 2: `keycode`: Código numérico de la tecla según estándar Linux input (ej: 30 para 'a')

**Respuesta exitosa (200 OK):**
```json
{
  "success": true,
  "message": "Tecla presionada correctamente",
  "key": "a",
  "keycode": 30
}
```

**Teclas especiales soportadas:**
- Enter (keycode 28)
- Space (keycode 57)
- Backspace (keycode 14)
- Tab (keycode 15)
- Esc (keycode 1)
- Flechas direccionales (Up: 103, Down: 108, Left: 105, Right: 106)

**Proceso interno:**
1. Valida el token de autenticación
2. Verifica permisos de `full_control`
3. Si se recibe `key`, lo convierte al keycode correspondiente
4. Invoca syscall `keyboard_caption` con el keycode
5. Retorna confirmación de la operación

---

## Protocolo WebSocket para Streaming

El WebSocket proporciona comunicación bidireccional en tiempo real para transmitir la pantalla y los recursos del sistema a los clientes conectados.

### Conexión al WebSocket

**URL:** `ws://10.150.1.233:8080/ws`  
**Protocolo:** WebSocket sobre HTTP

```javascript
// Ejemplo de conexión desde JavaScript
const ws = new WebSocket('ws://10.150.1.233:8080/ws');

ws.onopen = () => {
    console.log('Conectado al servidor');
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    
    if (data.type === 'screenshot') {
        // Procesar imagen Base64
        const img = document.getElementById('screen');
        img.src = 'data:image/jpeg;base64,' + data.data;
    } else if (data.type === 'resources') {
        // Procesar información de recursos
        console.log('CPU:', data.cpu_percent + '%');
        console.log('RAM:', data.ram_percent + '%');
    }
};

ws.onclose = () => {
    console.log('Desconectado del servidor');
};
```

### Arquitectura del WebSocket Handler

El `WebSocketHandler` implementa un sistema de streaming mediante dos threads independientes:

```cpp
void WebSocketHandler::start() {
    running_ = true;
    
    // Thread 1: Captura y envío de pantalla
    screenshot_thread_ = std::thread(&WebSocketHandler::screenshotLoop, this);
    
    // Thread 2: Captura y envío de recursos
    resources_thread_ = std::thread(&WebSocketHandler::resourcesLoop, this);
}
```

### Thread de Captura de Pantalla (screenshotLoop)

Este thread ejecuta un bucle continuo que captura la pantalla y la transmite a todos los clientes conectados:

```cpp
void WebSocketHandler::screenshotLoop() {
    while (running_) {
        // 1. Capturar pantalla mediante syscall
        std::string base64_image = Syscalls::getScreenshotBase64();
        
        if (!base64_image.empty()) {
            // 2. Crear mensaje JSON
            crow::json::wvalue json_msg;
            json_msg["type"] = "screenshot";
            json_msg["data"] = base64_image;
            json_msg["timestamp"] = std::chrono::system_clock::now()
                                    .time_since_epoch().count();
            
            std::string message = json_msg.dump();
            
            // 3. Enviar a todos los clientes
            broadcast(message);
        }
        
        // 4. Esperar según FPS configurado (1 FPS = 1000ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / Config::FPS));
    }
}
```

**Proceso detallado:**

1. **Captura de pantalla**: Invoca `getScreenshotBase64()` que:
   - Llama a la syscall `screen_live`
   - Obtiene el framebuffer raw
   - Codifica la imagen en Base64
   - Retorna la cadena Base64

2. **Construcción del mensaje JSON**:
   ```json
   {
     "type": "screenshot",
     "data": "/9j/4AAQSkZJRgABAQEA...",
     "timestamp": 1234567890123456
   }
   ```

3. **Broadcasting**: Envía el mensaje a todos los clientes conectados mediante `broadcast()`

4. **Control de framerate**: Pausa calculada según la configuración de FPS
   - Si FPS = 1: pausa de 1000ms (1 segundo por frame)
   - Si FPS = 30: pausa de 33ms (30 frames por segundo)

### Thread de Recursos del Sistema (resourcesLoop)

Este thread captura periódicamente el uso de CPU y RAM:

```cpp
void WebSocketHandler::resourcesLoop() {
    while (running_) {
        // 1. Obtener recursos mediante syscall
        std::string resources_json = Syscalls::getResourcesJSON();
        
        if (!resources_json.empty()) {
            // 2. Enviar a todos los clientes
            broadcast(resources_json);
        }
        
        // 3. Actualizar cada 2 segundos
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
```

**Formato del mensaje de recursos:**
```json
{
  "type": "resources",
  "cpu_percent": 45,
  "ram_percent": 62,
  "total_ram_mb": 8192,
  "used_ram_mb": 5079,
  "free_ram_mb": 3113,
  "timestamp": 1234567890
}
```

**Frecuencia de actualización:** 2 segundos (menos frecuente que las capturas de pantalla para reducir carga)

### Gestión de Conexiones

El handler mantiene un set de conexiones activas protegido por mutex:

```cpp
void WebSocketHandler::addConnection(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.insert(&conn);
    std::cout << "Nueva conexión. Total: " << connections_.size() << std::endl;
}

void WebSocketHandler::removeConnection(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(&conn);
    std::cout << "Conexión cerrada. Total: " << connections_.size() << std::endl;
}
```

### Función de Broadcasting

Envía un mensaje a todas las conexiones activas de forma segura:

```cpp
void WebSocketHandler::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    for (auto* conn : connections_) {
        try {
            conn->send_text(message);
        } catch (const std::exception& e) {
            std::cerr << "Error al enviar mensaje: " << e.what() << std::endl;
        }
    }
}
```

**Características:**
- Thread-safe mediante mutex
- Manejo de excepciones para conexiones fallidas
- Envío a múltiples clientes simultáneamente

### Mensajes del Cliente al Servidor

Los clientes pueden enviar comandos de control:

```javascript
// Iniciar streaming
ws.send(JSON.stringify({
    command: "start_stream"
}));

// Pausar streaming
ws.send(JSON.stringify({
    command: "stop_stream"
}));
```

**Procesamiento de mensajes:**
```cpp
void WebSocketHandler::handleMessage(crow::websocket::connection& conn, 
                                     const std::string& message) {
    auto json_msg = crow::json::load(message);
    
    if (json_msg && json_msg.has("command")) {
        std::string command = json_msg["command"].s();
        
        if (command == "start_stream") {
            // Iniciar o reanudar streaming
        } else if (command == "stop_stream") {
            // Pausar streaming
        }
    }
}
```

### Ciclo de Vida del WebSocket

```
Cliente                          Servidor
  |                                 |
  |------- Conexión HTTP ---------> |
  |<------ Upgrade WebSocket ------- |
  |                                 |
  |===== Conexión establecida ======|
  |                                 |
  |<----- Mensaje (screenshot) ----- |
  |<----- Mensaje (resources) ------ |
  |                                 |
  |---- Comando (start/stop) -----> |
  |                                 |
  |<----- Mensaje (screenshot) ----- |
  |                                 |
  |------- Close Connection ------> |
  |                                 |
```

---

## Pruebas con Thunder Client

Thunder Client es una extensión de Visual Studio Code que permite realizar peticiones HTTP de forma similar a Postman.

### Configuración Inicial

1. **Instalar Thunder Client** en VS Code desde el marketplace
2. **Configurar URL base**: `http://10.150.1.233:8080`
3. **Crear colección**: "USAC Linux Remote Desktop API"

### Prueba 1: Verificación del Servidor

**Endpoint:** `GET /health`

**Configuración en Thunder Client:**
- Method: GET
- URL: `http://10.150.1.233:8080/health`
- Headers: Ninguno
- Body: Ninguno

**Resultado esperado:**
- Status: 200 OK
- Body:
  ```json
  {
    "status": "ok",
    "message": "Servidor funcionando correctamente"
  }
  ```

**Validación:** Confirma que el servidor está activo y respondiendo correctamente.

### Prueba 2: Login de Usuario

**Endpoint:** `POST /api/auth/login`

**Configuración en Thunder Client:**
- Method: POST
- URL: `http://10.150.1.233:8080/api/auth/login`
- Headers:
  - `Content-Type: application/json`
- Body (JSON):
  ```json
  {
    "username": "admin",
    "password": "123"
  }
  ```

**Resultado esperado:**
- Status: 200 OK
- Body:
  ```json
  {
    "success": true,
    "username": "admin",
    "access_level": "full_control",
    "token": "eyJhbGciOiJIUzI1NiIs...",
    "groups": ["admin", "remote_control"],
    "can_view": true,
    "can_control": true
  }
  ```

**Validación:**
- El token debe ser una cadena alfanumérica larga
- `can_control` debe ser `true` para usuarios del grupo `remote_control`
- Copiar el token para usar en pruebas posteriores

### Prueba 3: Login con Usuario sin Permisos

**Configuración:**
- Body:
  ```json
  {
    "username": "guest",
    "password": "guest123"
  }
  ```

**Resultado esperado:**
- Status: 403 Forbidden
- Body:
  ```json
  {
    "success": false,
    "error": "Usuario no tiene permisos para acceso remoto"
  }
  ```

**Validación:** Confirma que el sistema de permisos basado en grupos funciona correctamente.

### Prueba 4: Click del Mouse

**Endpoint:** `POST /api/mouse/click`

**Configuración en Thunder Client:**
- Method: POST
- URL: `http://10.150.1.233:8080/api/mouse/click`
- Headers:
  - `Content-Type: application/json`
  - `Authorization: Bearer eyJhbGciOiJIUzI1NiIs...` (token del login)
- Body (JSON):
  ```json
  {
    "x": 640,
    "y": 400,
    "button": 1
  }
  ```

**Resultado esperado:**
- Status: 200 OK
- Body:
  ```json
  {
    "success": true,
    "message": "Click ejecutado correctamente"
  }
  ```

**Validación:**
- Observar en el sistema que el cursor se mueve a (640, 400)
- Verificar que se ejecuta un click izquierdo
- Revisar logs del kernel: `dmesg | grep mouse`

### Prueba 5: Presión de Tecla por Carácter

**Endpoint:** `POST /api/keyboard/press`

**Configuración:**
- Body:
  ```json
  {
    "key": "h"
  }
  ```

**Resultado esperado:**
- Status: 200 OK
- La tecla 'h' debe escribirse en la aplicación activa

**Validación:** Abrir un editor de texto y verificar que el carácter aparece.

### Prueba 6: Presión de Tecla Especial

**Configuración:**
- Body:
  ```json
  {
    "key": "Enter"
  }
  ```

**Resultado esperado:**
- Status: 200 OK
- Se ejecuta un "Enter" en la aplicación activa

### Prueba 7: Acceso sin Token

**Endpoint:** `POST /api/mouse/click`

**Configuración:**
- Headers: Sin `Authorization`
- Body: Cualquier payload válido

**Resultado esperado:**
- Status: 401 Unauthorized
- Body:
  ```json
  {
    "success": false,
    "error": "Token de autenticación requerido"
  }
  ```

**Validación:** Confirma que los endpoints protegidos no son accesibles sin autenticación.

### Prueba 8: Conexión WebSocket

Para probar el WebSocket se puede usar una herramienta como:
- **websocat** (línea de comandos)
- **WebSocket King Client** (Chrome extension)
- **Cliente JavaScript personalizado**

**Ejemplo con websocat:**
```bash
websocat ws://10.150.1.233:8080/ws
```

**Resultado esperado:**
- Conexión establecida exitosamente
- Recepción continua de mensajes JSON con tipo "screenshot" y "resources"
- Mensajes cada ~1 segundo (screenshot) y ~2 segundos (resources)

**Validación:**
- Decodificar una imagen Base64 y verificar que es la pantalla actual
- Verificar que los porcentajes de CPU y RAM son consistentes con el uso real del sistema

---

## Resumen del Flujo de Comunicación

```
[Cliente Frontend]
       |
       | 1. POST /api/auth/login
       |    (username, password)
       v
[Backend API]
       |
       | 2. Validación PAM
       v
[Sistema Linux]
       |
       | 3. Token generado
       v
[Cliente Frontend]
       |
       | 4. WebSocket Connection
       |    (con token)
       v
[WebSocket Handler]
       |
       | 5. Streaming continuo
       |    (screenshots + resources)
       v
[Cliente Frontend]
       |
       | 6. POST /api/mouse/click
       |    POST /api/keyboard/press
       |    (con token + Authorization)
       v
[Backend API]
       |
       | 7. Validación permisos
       | 8. Invocación syscalls
       v
[Kernel Modificado]
       |
       | 9. Ejecución de acciones
       v
[Sistema Linux]
```

Este sistema de endpoints y WebSocket proporciona una arquitectura completa para el control remoto del escritorio Linux con autenticación robusta y streaming en tiempo real.



### Implementación de Seguridad con PAM

En esta parte se describe la integración del sistema de autenticación PAM (Pluggable Authentication Modules) en el backend. Se explica cómo se configuró PAM para validar las credenciales de los usuarios del sistema Linux, los archivos de configuración modificados, y cómo se implementó la lógica de autenticación en el código del backend. Se detalla el flujo de autenticación desde que el usuario envía sus credenciales hasta que se valida con PAM.


Perfecto. A continuación tienes una redacción formal y completa para los dos apartados de tu documentación, basándose en la información que proporcionaste. Está escrita con un tono técnico y profesional, ideal para incluir en un informe o memoria de proyecto.

### Configuración del Sistema Linux

Para habilitar el uso de PAM dentro del backend, fue necesario instalar las bibliotecas de desarrollo correspondientes mediante el siguiente comando:

```bash
sudo apt-get update
sudo apt-get install -y libpam0g-dev
```

Estas bibliotecas permiten que el programa pueda interactuar directamente con los módulos de autenticación del sistema operativo.

### Integración de PAM en el Backend

El backend se desarrolló con soporte para autenticación a nivel del sistema operativo utilizando PAM. Cuando un usuario intenta iniciar sesión, el servidor recibe sus credenciales (nombre de usuario y contraseña) y las valida mediante las funciones proporcionadas por la biblioteca PAM.

El flujo de autenticación es el siguiente:

1. **El cliente envía sus credenciales** al backend mediante una solicitud HTTP.
2. **El backend inicia un contexto PAM**, indicando el servicio de autenticación configurado.
3. **Se envían las credenciales recibidas** al módulo PAM.
4. **PAM verifica las credenciales** contra los usuarios locales del sistema Linux.
5. Si la validación es exitosa, el backend continúa con la sesión y asigna los permisos correspondientes.
6. En caso contrario, se devuelve un mensaje de error y se deniega el acceso.

```bash
┌─────────────┐
│ Backend     │
│   (C++)     │
└──────┬──────┘
       │
       │ pam_authenticate()
       ↓
┌─────────────┐
│   PAM API   │ ← Librería libpam
└──────┬──────┘
       │
       │ Consulta archivos de configuración
       ↓
┌─────────────────────────────┐
│ /etc/pam.d/common-auth      │ ← Configuración PAM
│ /etc/passwd                 │ ← Usuarios del sistema
│ /etc/shadow                 │ ← Contraseñas encriptadas
│ /etc/group                  │ ← Grupos del sistema
└─────────────────────────────┘

**Flujo de autenticación:**
1. Usuario envía username + password al backend
2. Backend llama a `pam_authenticate(username, password)`
3. PAM verifica las credenciales contra `/etc/shadow`
4. PAM retorna éxito o fallo
5. Backend consulta los grupos del usuario con `getgrgid()`
6. Backend determina permisos (remote_view o remote_control)
```

### Configuración de Permisos del Ejecutable

Debido a que la autenticación mediante PAM requiere permisos elevados, el ejecutable del servidor necesita ejecutarse con privilegios administrativos. Esto puede realizarse de dos formas:

**Modo de desarrollo:**

```bash
sudo ./servidor
```

**Modo de producción:**

```bash
sudo setcap cap_dac_read_search+ep ./servidor
```

El uso de `setcap` permite otorgar los permisos mínimos necesarios al binario sin requerir que se ejecute directamente como superusuario, incrementando así la seguridad del sistema.

### Compilación y Ejecución

Para compilar y ejecutar el servidor con soporte PAM, se siguen los siguientes pasos:

```bash
cd ~/Documentos/sopes2/proyecto_sopes2_2S25/backend

# Limpiar compilaciones previas
rm -rf build

# Generar nuevo entorno de compilación
mkdir build
cd build
cmake ..
make -j$(nproc)

# Ejecutar con permisos para PAM
sudo ./servidor
```

Con esto, el backend queda correctamente integrado con el sistema de autenticación de Linux mediante PAM, garantizando que solo usuarios válidos del sistema puedan acceder al servicio.


### Sistema de Control de Acceso Basado en Grupos

Aquí se documenta la implementación del sistema de permisos utilizando los grupos del sistema. Se explica la creación de los grupos `remote_view` y `remote_control`, cómo se asignan usuarios a estos grupos, y cómo el backend verifica la pertenencia a grupos para determinar los permisos de cada usuario. Se detalla la lógica implementada para diferenciar entre usuarios que solo pueden ver el escritorio y aquellos que pueden controlarlo.

### Creación de Grupos del Sistema

Se definieron dos grupos principales para diferenciar los niveles de acceso de los usuarios:

* **remote_view**: usuarios con permisos únicamente de visualización del escritorio remoto.
* **remote_control**: usuarios con permisos de control total sobre el escritorio remoto.

La creación de estos grupos se realizó mediante los siguientes comandos:

```bash
sudo groupadd remote_view
sudo groupadd remote_control
```

La correcta creación puede verificarse con:

```bash
getent group remote_view
getent group remote_control
```

### Asignación de Usuarios a Grupos

Cada usuario se asigna a un grupo de acuerdo con el tipo de acceso que debe poseer. A modo de ejemplo, se creó un usuario administrador con permisos completos:

```bash
sudo useradd -m -s /bin/bash admin
echo "admin:---" | sudo chpasswd
sudo usermod -aG remote_control admin
id admin
```

La salida del comando `id admin` debe mostrar la pertenencia al grupo `remote_control`.

Asimismo, se pueden crear usuarios adicionales con distintos niveles de acceso:

```bash
# Usuario con acceso solo de visualización
sudo useradd -m -s /bin/bash viewer
echo "viewer:viewer123" | sudo chpasswd
sudo usermod -aG remote_view viewer

# Usuario con acceso de control total
sudo useradd -m -s /bin/bash controller
echo "controller:controller123" | sudo chpasswd
sudo usermod -aG remote_control controller
```

### Lógica de Control de Acceso en el Backend

Una vez que el usuario ha sido autenticado correctamente mediante PAM, el backend realiza una verificación adicional para determinar a qué grupo pertenece. Esta comprobación se implementa mediante llamadas al sistema que consultan la lista de grupos asociados al usuario autenticado.

El comportamiento es el siguiente:

* Si el usuario pertenece al grupo **remote_view**, solo podrá **visualizar** el escritorio remoto.
* Si el usuario pertenece al grupo **remote_control**, tendrá permisos para **interactuar y controlar** el escritorio remoto.
* Si no pertenece a ninguno de los grupos, se **deniega el acceso** al sistema.

Esta separación de roles permite un control granular sobre las acciones que cada usuario puede realizar, reforzando la seguridad general del sistema.



### Desarrollo del Frontend con React

En esta parte se explica la creación de la interfaz web utilizando React. Se describe la estructura inicial del proyecto frontend, la organización de componentes, y la arquitectura general de la aplicación web. Se detalla cómo se configuró el proyecto, las dependencias principales utilizadas, y la estructura de carpetas adoptada.

### Implementación de Componentes Principales

Aquí se documentan los componentes React creados para la aplicación. Se explica la implementación del Header (encabezado), Body (contenido principal) y Footer (pie de página), así como cualquier otro componente estructural importante. Se describe la funcionalidad de cada componente y cómo se relacionan entre sí dentro de la aplicación.

### Integración del Frontend con el Backend

Esta sección detalla cómo se estableció la comunicación entre el frontend y el backend. Se explica la configuración de las peticiones HTTP desde React hacia los endpoints de la API, el manejo de respuestas asíncronas, y la gestión de estados en el frontend. Se documenta cómo se muestran los datos obtenidos del backend (monitoreo de CPU, RAM, escritorio) en la interfaz de usuario.

### Implementación del Sistema de Login

La creación de la pantalla de login en el frontend. Se explica cómo se capturan las credenciales del usuario, cómo se envían al endpoint de autenticación del backend, y cómo se maneja la respuesta de autenticación. Se detalla el manejo de sesiones en el cliente, el almacenamiento de tokens o información de sesión, y la lógica de redirección tras un login exitoso.

![login](images/login.png)

### Control de Acceso en el Frontend

Esta sección documenta cómo se implementó el control de acceso basado en roles en la interfaz web. Se explica cómo el frontend determina los permisos del usuario autenticado, cómo se habilitan o deshabilitan funcionalidades según el grupo al que pertenece el usuario (remote_view vs remote_control), y cómo se protegen las rutas o componentes que requieren permisos específicos.

### Arquitectura de Comunicación del Sistema

En esta parte se presenta una visión general de cómo se comunican los tres componentes principales del sistema. Se explica el flujo de datos desde que el usuario interactúa con el frontend, pasando por la API en el backend, hasta llegar a las syscalls del kernel y el retorno de información. Se incluyen diagramas que ilustran la arquitectura cliente-servidor implementada, incluyendo las direcciones IP y puertos utilizados para la comunicación entre el frontend en la PC física y el backend en la máquina virtual.


## Estructura del Frontend

```
frontend/
├── package.json                # Dependencias y scripts del proyecto React
├── vite.config.js              # Configuración de Vite para desarrollo y build
└── src/                        # Código fuente principal del frontend
    ├── App.jsx                 # Componente raíz que define la estructura general de la aplicación
    ├── App.css                 # Estilos globales asociados a App.jsx
    ├── index.css               # Estilos base y variables globales
    ├── main.jsx                # Punto de entrada; monta la aplicación React en el DOM
    ├── assets/                 # Recursos estáticos (imágenes, íconos, etc.)
    ├── components/             # Componentes reutilizables de la interfaz
    │   ├── Header.jsx          # Barra superior con navegación, nombre del sistema o sesión activa
    │   ├── Footer.jsx          # Pie de página con información del sistema o créditos
    │   ├── RemoteDesktop.jsx   # Componente principal que renderiza la vista del escritorio remoto
    │   └── SystemStats.jsx     # Módulo de monitoreo que muestra estadísticas de CPU, RAM, etc.
    ├── contexts/               # Contextos globales de React
    │   └── AutoContext.jsx     # Contexto global para manejo del estado de autenticación (usuario activo, tokens)
    ├── hooks/                  # Hooks personalizados para lógica reutilizable
    │   ├── useAuth.js          # Hook que gestiona inicio/cierre de sesión y persistencia del usuario
    │   └── useWebSocket.js     # Hook que administra la conexión WebSocket (envío y recepción de datos en tiempo real)
    ├── pages/                  # Páginas principales de la aplicación
    │   ├── LoginPage.jsx       # Página de inicio de sesión; solicita credenciales y las envía al backend
    │   ├── RegisterPage.jsx    # Página de registro de nuevos usuarios (si el sistema lo permite)
    │   └── MainPage.jsx        # Página principal tras autenticación; integra RemoteDesktop y SystemStats
    └── services/               # Servicios para comunicación con el backend
        ├── apiService.js       # Encapsula las peticiones HTTP (login, registro, etc.) mediante Axios o Fetch
        └── websocketService.js # Gestiona la creación, reconexión y eventos del canal WebSocket
```

---

## Descripción General del Frontend

El frontend fue desarrollado utilizando **React** junto con el bundler **Vite**, lo que proporciona un entorno rápido y modular para la construcción de interfaces web.
Su objetivo principal es ofrecer una interfaz de usuario moderna y reactiva para la interacción con el backend, incluyendo autenticación, visualización remota y monitoreo del sistema.

### Arquitectura

El diseño sigue una estructura **component-based** donde cada módulo de la interfaz (barra superior, consola remota, estadísticas, etc.) se implementa como un componente independiente.
El estado global de la aplicación se maneja mediante **Context API** y **hooks personalizados**, lo que facilita la comunicación entre componentes sin necesidad de prop drilling.

### Comunicación con el Backend

* Las peticiones HTTP (autenticación, registro, etc.) se canalizan a través del módulo `apiService.js`.
* La comunicación en tiempo real (captura de pantalla y control remoto) se realiza mediante WebSocket, gestionado por `useWebSocket.js` y `websocketService.js`.
* Todos los endpoints y configuraciones se centralizan en estos servicios para mantener la lógica de red desacoplada de los componentes visuales.

### Flujo de la Aplicación

1. **LoginPage.jsx**: el usuario ingresa sus credenciales; se valida con el backend mediante PAM.
2. **AutoContext.jsx / useAuth.js**: almacenan el estado del usuario autenticado.
3. **MainPage.jsx**: carga los componentes principales tras autenticación.
4. **RemoteDesktop.jsx**: establece una conexión WebSocket para recibir las capturas del escritorio remoto en tiempo real.
5. **SystemStats.jsx**: obtiene y muestra métricas del sistema enviadas desde el backend.

