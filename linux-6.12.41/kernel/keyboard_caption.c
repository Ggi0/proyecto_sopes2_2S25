#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

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
 *   -ENODEV si no se encuentra dispositivo de teclado
 *   -EINVAL si el keycode es inválido
 * 
 * Funcionamiento:
 *   Simula una pulsación completa de tecla:
 *   1. Presionar tecla (keydown)
 *   2. Esperar brevemente
 *   3. Soltar tecla (keyup)
 * 
 * IMPORTANTE: 
 *   - El keycode debe venir ya convertido desde el backend (API hace el mapeo)
 *   - Frontend envía nombre de tecla ("a"), API lo convierte a keycode (30)
 *   - Esta syscall solo simula la presión física de la tecla
 */
SYSCALL_DEFINE1(keyboard_caption, int, keycode)
{
    struct input_dev *kbd_dev = NULL;
    struct input_handle *handle;
    struct input_handler *handler;
    int found = 0;
    
    // Validar que el keycode esté en rango válido
    // KEY_MAX está definido en <linux/input-event-codes.h>
    // Típicamente es ~767 en kernels modernos
    if (keycode < 1 || keycode > KEY_MAX) {
        printk(KERN_WARNING "keyboard_caption: Keycode inválido: %d (rango: 1-%d)\n", 
               keycode, KEY_MAX);
        return -EINVAL;
    }
    
    /*
     * BÚSQUEDA DE DISPOSITIVO DE TECLADO
     * 
     * Recorremos el input subsystem del kernel buscando un dispositivo
     * que sea un teclado real o virtual.
     * 
     * Estrategia:
     * 1. Buscar handlers de tipo "kbd" o "evdev"
     * 2. Verificar que el dispositivo soporte eventos de teclado (EV_KEY)
     * 3. Verificar que tenga al menos una tecla alfanumérica (KEY_A)
     */
    list_for_each_entry(handler, &input_handler_list, node) {
        // Buscamos handlers que manejen teclados
        // "kbd" es el handler específico de teclado
        // "evdev" es un handler genérico que también puede manejar teclados
        if (strcmp(handler->name, "kbd") == 0 || 
            strcmp(handler->name, "evdev") == 0) {
            
            // Recorremos los dispositivos conectados a este handler
            list_for_each_entry(handle, &handler->h_list, h_node) {
                kbd_dev = handle->dev;
                
                /*
                 * Verificaciones para confirmar que es un teclado:
                 * 
                 * 1. test_bit(EV_KEY, kbd_dev->evbit)
                 *    - Verifica que soporte eventos de teclas
                 * 
                 * 2. test_bit(KEY_A, kbd_dev->keybit)
                 *    - Verifica que tenga al menos la tecla 'A'
                 *    - Esto descarta dispositivos como mouse que también tienen EV_KEY
                 *      (para sus botones) pero no son teclados
                 */
                if (test_bit(EV_KEY, kbd_dev->evbit) && 
                    test_bit(KEY_A, kbd_dev->keybit)) {
                    found = 1;
                    break;
                }
            }
            
            if (found)
                break;
        }
    }
    
    // Si no encontramos ningún teclado, retornar error
    if (!found || !kbd_dev) {
        printk(KERN_ERR "keyboard_caption: No se encontró dispositivo de teclado\n");
        return -ENODEV;
    }
    
    /*
     * Verificar que el dispositivo soporte el keycode específico
     * 
     * Algunos dispositivos pueden no soportar todas las teclas
     * Por ejemplo, un teclado numérico no tiene letras
     */
    if (!test_bit(keycode, kbd_dev->keybit)) {
        printk(KERN_WARNING "keyboard_caption: Dispositivo no soporta keycode %d\n", keycode);
        // No retornamos error aquí, intentamos enviar de todos modos
        // En la práctica, la mayoría de teclados virtuales soportan todo
    }
    
    /*
     * FASE 1: PRESIONAR LA TECLA (keydown)
     * 
     * input_report_key: Reporta un evento de tecla al sistema
     * Parámetros:
     *   - kbd_dev: dispositivo de entrada
     *   - keycode: código de la tecla
     *   - 1: valor 1 = tecla presionada (keydown)
     */
    input_report_key(kbd_dev, keycode, 1);
    
    /*
     * input_sync: CRÍTICO
     * Sincroniza y despacha todos los eventos acumulados
     * Sin esto, el evento NO se procesará
     */
    input_sync(kbd_dev);
    
    printk(KERN_INFO "keyboard_caption: Tecla %d PRESIONADA\n", keycode);
    
    /*
     * PAUSA ENTRE KEYDOWN Y KEYUP
     * 
     * Simulamos el tiempo que un humano mantendría la tecla presionada
     * 
     * msleep: Pausa en milisegundos
     * - Si usamos muy poco tiempo: Algunos programas pueden ignorar la tecla
     * - Si usamos mucho tiempo: Puede sentirse lento para el usuario
     * - 50ms es un buen balance (similar a velocidad de escritura humana)
     * 
     * ALTERNATIVA: usleep_range(40000, 60000) para mayor precisión
     */
    msleep(50); // 50 milisegundos = 0.05 segundos
    
    /*
     * FASE 2: SOLTAR LA TECLA (keyup)
     * 
     * input_report_key con valor 0 = tecla soltada
     * Esto completa el ciclo de pulsación de la tecla
     */
    input_report_key(kbd_dev, keycode, 0); // 0 = soltada (keyup)
    input_sync(kbd_dev);
    
    printk(KERN_INFO "keyboard_caption: Tecla %d SOLTADA\n", keycode);
    
    // Log de éxito con información útil para debugging
    printk(KERN_INFO "keyboard_caption: Pulsación completa de keycode=%d exitosa\n", keycode);
    
    return 0; // Éxito
}