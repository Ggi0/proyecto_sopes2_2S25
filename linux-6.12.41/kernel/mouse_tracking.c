#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>

SYSCALL_DEFINE2(mouse_tracking, int, x, int, y)
{
    struct input_dev *mouse_dev = NULL;
    int err;
    
    // Validar coordenadas
    if (x < 0 || y < 0) {
        return -EINVAL;
    }
    
    // Crear dispositivo virtual de mouse
    mouse_dev = input_allocate_device();
    if (!mouse_dev) {
        return -ENOMEM;
    }
    
    // Configurar el dispositivo
    mouse_dev->name = "Virtual Mouse Syscall";
    mouse_dev->id.bustype = BUS_VIRTUAL;
    
    // Habilitar movimiento ABSOLUTO
    set_bit(EV_ABS, mouse_dev->evbit);
    set_bit(ABS_X, mouse_dev->absbit);
    set_bit(ABS_Y, mouse_dev->absbit);
    set_bit(EV_SYN, mouse_dev->evbit);
    
    // Configurar rangos de coordenadas (ajusta según tu resolución)
    input_set_abs_params(mouse_dev, ABS_X, 0, 1406, 0, 0);  // 0 a 1920 para Full HD
    input_set_abs_params(mouse_dev, ABS_Y, 0, 738, 0, 0);  // 0 a 1080 para Full HD
    
    // Registrar el dispositivo
    err = input_register_device(mouse_dev);
    if (err) {
        input_free_device(mouse_dev);
        return -ENODEV;
    }
    
    // MOVER el mouse a la posición absoluta
    input_report_abs(mouse_dev, ABS_X, x);
    input_report_abs(mouse_dev, ABS_Y, y);
    input_sync(mouse_dev);
    
    // Limpiar
    input_unregister_device(mouse_dev);
    
    return 0;
}