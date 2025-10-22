#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

/* 
 * SYSCALL: mouse_tracking
 * Propósito: Mover el cursor del mouse a coordenadas ABSOLUTAS específicas
 * 
 * Parámetros:
 *   @x: Coordenada X absoluta en la pantalla (0 a ancho_pantalla)
 *   @y: Coordenada Y absoluta en la pantalla (0 a alto_pantalla)
 * 
 * Retorno:
 *   0 en éxito
 *   -ENODEV si no se encuentra dispositivo de mouse
 *   -EINVAL si las coordenadas son inválidas
 * 
 * Nota: Esta syscall simula un movimiento ABSOLUTO, no relativo
 * Las coordenadas vienen ya convertidas desde el frontend (coordenadas reales del escritorio)
 */
SYSCALL_DEFINE2(mouse_tracking, int, x, int, y)
{
    struct input_dev *mouse_dev = NULL;
    struct input_handle *handle;
    struct input_handler *handler;
    int found = 0;
    
    // Validar que las coordenadas no sean negativas
    if (x < 0 || y < 0) {
        printk(KERN_WARNING "mouse_tracking: Coordenadas inválidas x=%d, y=%d\n", x, y);
        return -EINVAL;
    }
    
    // Buscar un dispositivo de mouse en el sistema de entrada
    // Recorremos todos los handlers registrados en el input subsystem
    list_for_each_entry(handler, &input_handler_list, node) {
        // Verificamos si el handler es del tipo "mouse" o "mousedev"
        if (strcmp(handler->name, "mousedev") == 0 || 
            strcmp(handler->name, "evdev") == 0) {
            
            // Recorremos todos los handles (conexiones) de este handler
            list_for_each_entry(handle, &handler->h_list, h_node) {
                mouse_dev = handle->dev;
                
                // Verificamos que el dispositivo soporte eventos de mouse
                // EV_REL = Eventos relativos (movimiento)
                // EV_ABS = Eventos absolutos (posición exacta)
                // EV_KEY = Eventos de teclas (botones del mouse)
                if (test_bit(EV_KEY, mouse_dev->evbit) && 
                    test_bit(BTN_LEFT, mouse_dev->keybit)) {
                    found = 1;
                    break;
                }
            }
            
            if (found)
                break;
        }
    }
    
    // Si no encontramos ningún dispositivo de mouse, retornar error
    if (!found || !mouse_dev) {
        printk(KERN_ERR "mouse_tracking: No se encontró dispositivo de mouse\n");
        return -ENODEV;
    }
    
    // Verificar si el dispositivo soporta eventos absolutos
    // La mayoría de sistemas modernos usan eventos absolutos para escritorio remoto
    if (test_bit(EV_ABS, mouse_dev->evbit)) {
        /*
         * MOVIMIENTO ABSOLUTO
         * Enviamos la posición exacta X, Y donde queremos que esté el cursor
         * Esto es ideal para escritorio remoto porque:
         * - El frontend sabe exactamente dónde hizo click el usuario
         * - No hay acumulación de errores
         * - Más preciso que movimientos relativos
         */
        input_report_abs(mouse_dev, ABS_X, x);
        input_report_abs(mouse_dev, ABS_Y, y);
        
        printk(KERN_INFO "mouse_tracking: Mouse movido (ABSOLUTO) a X=%d, Y=%d\n", x, y);
    } 
    else if (test_bit(EV_REL, mouse_dev->evbit)) {
        /*
         * FALLBACK: MOVIMIENTO RELATIVO
         * Si el dispositivo no soporta eventos absolutos, usamos relativos
         * 
         * PROBLEMA: No podemos mover a una posición absoluta directamente
         * SOLUCIÓN: Movemos relativamente asumiendo que empezamos en (0,0)
         * 
         * NOTA: Esto es menos preciso, pero funciona en algunos sistemas
         * En la práctica, la mayoría de sistemas virtuales soportan absolutos
         */
        input_report_rel(mouse_dev, REL_X, x);
        input_report_rel(mouse_dev, REL_Y, y);
        
        printk(KERN_INFO "mouse_tracking: Mouse movido (RELATIVO) dx=%d, dy=%d\n", x, y);
    }
    else {
        // El dispositivo no soporta ni absoluto ni relativo, error
        printk(KERN_ERR "mouse_tracking: Dispositivo no soporta movimiento\n");
        return -EINVAL;
    }
    
    /*
     * input_sync: CRÍTICO
     * Esta función "despacha" todos los eventos acumulados al sistema
     * Sin esto, el movimiento NO se aplicará
     * Es como hacer "commit" de los cambios
     */
    input_sync(mouse_dev);
    
    return 0; // Éxito
}