#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/mutex.h>

static struct input_dev *global_click_dev = NULL;
static DEFINE_MUTEX(click_lock);

static int init_click_device(void)
{
    int err;
    
    if (global_click_dev != NULL)
        return 0; // Ya está inicializado
    
    global_click_dev = input_allocate_device();
    if (!global_click_dev) {
        pr_err("mouse_action: No se pudo alocar dispositivo\n");
        return -ENOMEM;
    }
    
    // Configurar el dispositivo
    global_click_dev->name = "Syscall Virtual Mouse Clicks";
    global_click_dev->id.bustype = BUS_USB;
    global_click_dev->id.vendor  = 0x1234;
    global_click_dev->id.product = 0x5679;
    global_click_dev->id.version = 0x0100;
    
    // Habilitar eventos de botones
    set_bit(EV_KEY, global_click_dev->evbit);
    set_bit(BTN_LEFT, global_click_dev->keybit);
    set_bit(BTN_RIGHT, global_click_dev->keybit);
    set_bit(BTN_MIDDLE, global_click_dev->keybit);
    
    // Habilitar también movimiento relativo (para que sea reconocido como mouse)
    set_bit(EV_REL, global_click_dev->evbit);
    set_bit(REL_X, global_click_dev->relbit);
    set_bit(REL_Y, global_click_dev->relbit);
    
    // Registrar el dispositivo
    err = input_register_device(global_click_dev);
    if (err) {
        pr_err("mouse_action: No se pudo registrar dispositivo\n");
        input_free_device(global_click_dev);
        global_click_dev = NULL;
        return -ENODEV;
    }
    
    pr_info("mouse_action: Dispositivo virtual de clicks creado exitosamente\n");
    return 0;
}

SYSCALL_DEFINE1(mouse_action, int, button)
{
    int button_code;
    int ret;
    
    // Validar botón: 1=izquierdo, 2=derecho
    if (button == 1) {
        button_code = BTN_LEFT;
    } else if (button == 2) {
        button_code = BTN_RIGHT;
    } else {
        pr_warn("mouse_action: Botón inválido: %d\n", button);
        return -EINVAL;
    }
    
    mutex_lock(&click_lock);
    
    // Inicializar el dispositivo si no existe
    ret = init_click_device();
    if (ret != 0) {
        mutex_unlock(&click_lock);
        return ret;
    }
    
    // Hacer el click: presionar
    input_report_key(global_click_dev, button_code, 1);
    input_sync(global_click_dev);
    
    // Pequeña pausa
    msleep(50);
    
    // Soltar
    input_report_key(global_click_dev, button_code, 0);
    input_sync(global_click_dev);
    
    mutex_unlock(&click_lock);
    
    pr_info("mouse_action: Click %s ejecutado\n", 
            button == 1 ? "izquierdo" : "derecho");
    return 0;
}