#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

SYSCALL_DEFINE1(mouse_action, int, button)
{
    struct input_dev *mouse_dev = NULL;
    int button_code;
    
    // Validar botón: 1=izquierdo, 2=derecho
    if (button == 1) {
        button_code = BTN_LEFT;
    } else if (button == 2) {
        button_code = BTN_RIGHT;
    } else {
        return -EINVAL;
    }
    
    // Crear dispositivo virtual de mouse
    mouse_dev = input_allocate_device();
    if (!mouse_dev) {
        return -ENOMEM;
    }
    
    // Configurar el dispositivo
    mouse_dev->name = "Virtual Mouse Click";
    mouse_dev->id.bustype = BUS_VIRTUAL;
    
    // Habilitar eventos de botones
    set_bit(EV_KEY, mouse_dev->evbit);
    set_bit(button_code, mouse_dev->keybit);
    set_bit(EV_SYN, mouse_dev->evbit);
    
    // Registrar el dispositivo
    if (input_register_device(mouse_dev)) {
        input_free_device(mouse_dev);
        return -ENODEV;
    }
    
    // Hacer el click: presionar y soltar
    input_report_key(mouse_dev, button_code, 1);  // Presionar
    input_sync(mouse_dev);
    
    msleep(50);  // Pausa de 50ms
    
    input_report_key(mouse_dev, button_code, 0);  // Soltar
    input_sync(mouse_dev);
    
    // Limpiar
    input_unregister_device(mouse_dev);
    input_free_device(mouse_dev);
    
    return 0;
}