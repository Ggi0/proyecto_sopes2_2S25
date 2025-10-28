#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/mutex.h>

static struct input_dev *global_mouse_dev = NULL;
static DEFINE_MUTEX(mouse_lock);

static int init_mouse_device(void)
{
    int err;
    
    if (global_mouse_dev != NULL)
        return 0; // Ya está inicializado
    
    global_mouse_dev = input_allocate_device();
    if (!global_mouse_dev) {
        pr_err("mouse_tracking: No se pudo alocar dispositivo\n");
        return -ENOMEM;
    }
    
    // Configurar el dispositivo
    global_mouse_dev->name = "Syscall Virtual Mouse";
    global_mouse_dev->id.bustype = BUS_USB; // Usar BUS_USB en lugar de BUS_VIRTUAL
    global_mouse_dev->id.vendor  = 0x1234;
    global_mouse_dev->id.product = 0x5678;
    global_mouse_dev->id.version = 0x0100;
    
    // Habilitar movimiento ABSOLUTO
    set_bit(EV_ABS, global_mouse_dev->evbit);
    set_bit(ABS_X, global_mouse_dev->absbit);
    set_bit(ABS_Y, global_mouse_dev->absbit);
    
    // Habilitar también clicks (para que sea reconocido como mouse completo)
    set_bit(EV_KEY, global_mouse_dev->evbit);
    set_bit(BTN_LEFT, global_mouse_dev->keybit);
    set_bit(BTN_RIGHT, global_mouse_dev->keybit);
    set_bit(BTN_MIDDLE, global_mouse_dev->keybit);
    
    // Configurar rangos para 1280x800
    input_set_abs_params(global_mouse_dev, ABS_X, 0, 1279, 0, 0);
    input_set_abs_params(global_mouse_dev, ABS_Y, 0, 799, 0, 0);
    
    // Registrar el dispositivo
    err = input_register_device(global_mouse_dev);
    if (err) {
        pr_err("mouse_tracking: No se pudo registrar dispositivo\n");
        input_free_device(global_mouse_dev);
        global_mouse_dev = NULL;
        return -ENODEV;
    }
    
    pr_info("mouse_tracking: Dispositivo virtual creado exitosamente\n");
    return 0;
}

SYSCALL_DEFINE2(mouse_tracking, int, x, int, y)
{
    int ret;
    
    // Validar coordenadas para 1280x800
    if (x < 0 || x >= 1280 || y < 0 || y >= 800) {
        pr_warn("mouse_tracking: Coordenadas fuera de rango: (%d, %d)\n", x, y);
        return -EINVAL;
    }
    
    mutex_lock(&mouse_lock);
    
    // Inicializar el dispositivo si no existe
    ret = init_mouse_device();
    if (ret != 0) {
        mutex_unlock(&mouse_lock);
        return ret;
    }
    
    // Enviar el evento de movimiento absoluto
    input_report_abs(global_mouse_dev, ABS_X, x);
    input_report_abs(global_mouse_dev, ABS_Y, y);
    input_sync(global_mouse_dev);
    
    mutex_unlock(&mouse_lock);
    
    pr_info("mouse_tracking: Mouse movido a (%d, %d)\n", x, y);
    return 0;
}