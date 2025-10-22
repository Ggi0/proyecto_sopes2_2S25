#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/mutex.h>

/* Dispositivo virtual global que persiste entre llamadas */
static struct input_dev *global_virtual_kbd = NULL;

/* Mutex para proteger el acceso al dispositivo global */
static DEFINE_MUTEX(kbd_caption_mutex);

/* Flag para indicar si el dispositivo ha sido inicializado */
static bool kbd_caption_initialized = false;

/**
 * init_virtual_keyboard_caption - Inicializa el dispositivo virtual del teclado
 * Return: 0 si es exitoso, código de error negativo si falla
 */
static int init_virtual_keyboard_caption(void)
{
    int err;
    int i;
    
    /* Si ya está inicializado, no hacemos nada */
    if (kbd_caption_initialized && global_virtual_kbd) {
        return 0;
    }

    /* Creamos un dispositivo de entrada virtual */
    global_virtual_kbd = input_allocate_device();
    if (!global_virtual_kbd) {
        printk(KERN_ERR "keyboard_caption: No se pudo allocar dispositivo virtual\n");
        return -ENOMEM;
    }

    /* Configuramos las propiedades del dispositivo virtual */
    global_virtual_kbd->name = "Virtual Keyboard Caption";
    global_virtual_kbd->phys = "syscall/kbd_caption0";
    global_virtual_kbd->id.bustype = BUS_VIRTUAL;
    global_virtual_kbd->id.vendor  = 0x0001;
    global_virtual_kbd->id.product = 0x0002;
    global_virtual_kbd->id.version = 0x0100;

    /* Habilitamos los tipos de eventos */
    __set_bit(EV_KEY, global_virtual_kbd->evbit);
    __set_bit(EV_SYN, global_virtual_kbd->evbit);

    /* Habilitamos todas las teclas posibles */
    for (i = 0; i <= KEY_MAX; i++) {
        __set_bit(i, global_virtual_kbd->keybit);
    }

    /* Registramos el dispositivo en el sistema */
    err = input_register_device(global_virtual_kbd);
    if (err) {
        printk(KERN_ERR "keyboard_caption: Error registrando dispositivo: %d\n", err);
        input_free_device(global_virtual_kbd);
        global_virtual_kbd = NULL;
        return err;
    }

    kbd_caption_initialized = true;
    printk(KERN_INFO "keyboard_caption: Dispositivo virtual inicializado exitosamente\n");
    
    return 0;
}

/* 
 * SYSCALL: keyboard_caption
 * Propósito: Simular la pulsación de una tecla en el sistema
 * 
 * Parámetros:
 *   @keycode: Código de la tecla a simular (según estándar Linux input)
 *             Ejemplo: KEY_A = 30, KEY_ENTER = 28, KEY_0 = 11
 * 
 * Retorno:
 *   0 en éxito
 *   -ENODEV si no se puede inicializar el dispositivo
 *   -EINVAL si el keycode es inválido
 */
SYSCALL_DEFINE1(keyboard_caption, int, keycode)
{
    int ret;
    
    /* Validar que el keycode esté en rango válido */
    if (keycode < 1 || keycode > KEY_MAX) {
        printk(KERN_WARNING "keyboard_caption: Keycode inválido: %d (rango: 1-%d)\n", 
               keycode, KEY_MAX);
        return -EINVAL;
    }
    
    /* Adquirimos el mutex para acceso exclusivo */
    mutex_lock(&kbd_caption_mutex);

    /* Inicializamos el dispositivo virtual si es necesario */
    ret = init_virtual_keyboard_caption();
    if (ret) {
        mutex_unlock(&kbd_caption_mutex);
        return ret;
    }

    /* Verificamos que el dispositivo esté disponible */
    if (!global_virtual_kbd) {
        printk(KERN_ERR "keyboard_caption: Dispositivo virtual no disponible\n");
        mutex_unlock(&kbd_caption_mutex);
        return -ENODEV;
    }

    /* FASE 1: PRESIONAR LA TECLA (keydown) */
    input_report_key(global_virtual_kbd, keycode, 1);
    input_sync(global_virtual_kbd);
    
    printk(KERN_INFO "keyboard_caption: Tecla %d PRESIONADA\n", keycode);
    
    /* PAUSA ENTRE KEYDOWN Y KEYUP */
    msleep(50);
    
    /* FASE 2: SOLTAR LA TECLA (keyup) */
    input_report_key(global_virtual_kbd, keycode, 0);
    input_sync(global_virtual_kbd);
    
    printk(KERN_INFO "keyboard_caption: Tecla %d SOLTADA\n", keycode);

    /* Liberamos el mutex */
    mutex_unlock(&kbd_caption_mutex);
    
    printk(KERN_INFO "keyboard_caption: Pulsación completa de keycode=%d exitosa\n", keycode);
    
    return 0;
}