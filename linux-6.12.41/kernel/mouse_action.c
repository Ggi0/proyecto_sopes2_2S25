#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

/* 
 * SYSCALL: mouse_action
 * Propósito: Simular clicks del mouse (izquierdo o derecho)
 * 
 * Parámetros:
 *   @button: Tipo de botón a presionar
 *            1 = Botón izquierdo (BTN_LEFT)
 *            2 = Botón derecho (BTN_RIGHT)
 *            3 = Botón central/rueda (BTN_MIDDLE) - opcional
 * 
 * Retorno:
 *   0 en éxito
 *   -ENODEV si no se encuentra dispositivo de mouse
 *   -EINVAL si el botón especificado es inválido
 * 
 * Funcionamiento:
 *   Simula la acción completa de un click:
 *   1. Presionar el botón (keydown)
 *   2. Esperar un momento breve
 *   3. Soltar el botón (keyup)
 * 
 * IMPORTANTE: Esta syscall solo hace el click
 * El movimiento a la posición correcta debe hacerse ANTES con mouse_tracking
 */
SYSCALL_DEFINE1(mouse_action, int, button)
{
    struct input_dev *mouse_dev = NULL;
    struct input_handle *handle;
    struct input_handler *handler;
    int found = 0;
    int button_code;
    
    // Validar el parámetro del botón
    // Mapear el número recibido al código de botón de Linux
    switch (button) {
        case 1:
            button_code = BTN_LEFT;  // Código 0x110 (272)
            printk(KERN_INFO "mouse_action: Click IZQUIERDO solicitado\n");
            break;
            
        case 2:
            button_code = BTN_RIGHT; // Código 0x111 (273)
            printk(KERN_INFO "mouse_action: Click DERECHO solicitado\n");
            break;
            
        case 3:
            button_code = BTN_MIDDLE; // Código 0x112 (274) - Botón central/rueda
            printk(KERN_INFO "mouse_action: Click CENTRAL solicitado\n");
            break;
            
        default:
            printk(KERN_WARNING "mouse_action: Botón inválido: %d (usar 1=izq, 2=der, 3=central)\n", button);
            return -EINVAL;
    }
    
    // Buscar dispositivo de mouse en el input subsystem
    // Este proceso es similar al de mouse_tracking
    list_for_each_entry(handler, &input_handler_list, node) {
        if (strcmp(handler->name, "mousedev") == 0 || 
            strcmp(handler->name, "evdev") == 0) {
            
            list_for_each_entry(handle, &handler->h_list, h_node) {
                mouse_dev = handle->dev;
                
                // Verificar que el dispositivo soporte botones de mouse
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
    
    if (!found || !mouse_dev) {
        printk(KERN_ERR "mouse_action: No se encontró dispositivo de mouse\n");
        return -ENODEV;
    }
    
    // Verificar que el dispositivo soporte el botón solicitado
    if (!test_bit(button_code, mouse_dev->keybit)) {
        printk(KERN_WARNING "mouse_action: Dispositivo no soporta el botón %d\n", button);
        return -EINVAL;
    }
    
    /*
     * SIMULAR CLICK COMPLETO
     * 
     * Un click real consiste en 3 eventos:
     * 1. KEYDOWN: El botón es presionado (valor = 1)
     * 2. SYNC: Sincronizar eventos
     * 3. Pequeña pausa (simular tiempo de presión humano)
     * 4. KEYUP: El botón es soltado (valor = 0)
     * 5. SYNC: Sincronizar eventos
     */
    
    // FASE 1: PRESIONAR el botón (keydown)
    input_report_key(mouse_dev, button_code, 1); // 1 = presionado
    input_sync(mouse_dev); // Despachar evento
    
    printk(KERN_INFO "mouse_action: Botón %d PRESIONADO\n", button);
    
    /*
     * PAUSA BREVE
     * Esperamos un momento para simular el tiempo que un humano
     * mantendría el botón presionado
     * 
     * usleep_range: Función de kernel para esperar microsegundos
     * Rango: 10ms a 20ms (10000-20000 microsegundos)
     * 
     * ¿Por qué esperar?
     * - Hace que el click sea más "natural"
     * - Algunos programas pueden ignorar clicks demasiado rápidos
     * - Evita doble-clicks accidentales
     */
    usleep_range(10000, 20000); // Esperar 10-20 milisegundos
    
    // FASE 2: SOLTAR el botón (keyup)
    input_report_key(mouse_dev, button_code, 0); // 0 = soltado
    input_sync(mouse_dev); // Despachar evento
    
    printk(KERN_INFO "mouse_action: Botón %d SOLTADO\n", button);
    
    // Log final de éxito
    printk(KERN_INFO "mouse_action: Click completado exitosamente\n");
    
    return 0; // Éxito
}