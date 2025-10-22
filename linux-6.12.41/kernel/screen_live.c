#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <drm/drm_device.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fourcc.h>

// Estructura que se compartirá con el espacio de usuario
struct screen_capture_info {
    unsigned int width;          // Ancho de la pantalla
    unsigned int height;         // Alto de la pantalla
    unsigned int bytes_per_pixel; // Bytes por píxel (3 para RGB)
    unsigned long buffer_size;    // Tamaño total del buffer
    unsigned char *data;          // Puntero al buffer de datos RGB
};

SYSCALL_DEFINE1(screen_live, struct screen_capture_info __user *, capture_info_user)
{
    struct screen_capture_info capture_info_kernel;
    struct drm_device *drm_dev = NULL;
    struct drm_crtc *crtc = NULL;
    struct drm_framebuffer *fb = NULL;
    struct drm_gem_object *gem_obj = NULL;
    void *vaddr = NULL;
    unsigned char *fb_data = NULL;
    unsigned char *rgb_buffer = NULL;
    unsigned long fb_size;
    int ret = 0;
    int i, j;
    
    // Verificar que el puntero del usuario sea válido
    if (!capture_info_user) {
        printk(KERN_ERR "screen_live: Invalid user pointer\n");
        return -EFAULT;
    }
    
    // Copiar la estructura desde el espacio de usuario
    if (copy_from_user(&capture_info_kernel, capture_info_user, 
                       sizeof(struct screen_capture_info))) {
        printk(KERN_ERR "screen_live: Failed to copy from user\n");
        return -EFAULT;
    }
    
    // Buscar el dispositivo DRM (card0)
    drm_dev = drm_dev_get_by_minor(0);  // card0 generalmente es minor 0
    if (!drm_dev) {
        printk(KERN_ERR "screen_live: DRM device not found\n");
        return -ENODEV;
    }
    
    // Obtener el CRTC activo (pantalla principal)
    drm_for_each_crtc(crtc, drm_dev) {
        if (crtc->primary && crtc->primary->fb) {
            fb = crtc->primary->fb;
            break;
        }
    }
    
    if (!fb) {
        printk(KERN_ERR "screen_live: No active framebuffer found\n");
        drm_dev_put(drm_dev);
        return -ENODEV;
    }
    
    // Obtener dimensiones del framebuffer
    capture_info_kernel.width = fb->width;
    capture_info_kernel.height = fb->height;
    capture_info_kernel.bytes_per_pixel = 3; // RGB888
    capture_info_kernel.buffer_size = capture_info_kernel.width * 
                                     capture_info_kernel.height * 
                                     capture_info_kernel.bytes_per_pixel;
    
    // Verificar que el buffer del usuario sea suficientemente grande
    fb_size = fb->height * fb->pitches[0];
    
    // Obtener el objeto GEM del framebuffer
    gem_obj = fb->obj[0];
    if (!gem_obj) {
        printk(KERN_ERR "screen_live: No GEM object in framebuffer\n");
        drm_dev_put(drm_dev);
        return -EINVAL;
    }
    
    // Mapear el framebuffer a memoria del kernel
    vaddr = drm_gem_vmap(gem_obj);
    if (!vaddr || IS_ERR(vaddr)) {
        printk(KERN_ERR "screen_live: Failed to map framebuffer\n");
        drm_dev_put(drm_dev);
        return -ENOMEM;
    }
    
    fb_data = (unsigned char *)vaddr;
    
    // Reservar buffer temporal para convertir a RGB
    rgb_buffer = kmalloc(capture_info_kernel.buffer_size, GFP_KERNEL);
    if (!rgb_buffer) {
        printk(KERN_ERR "screen_live: Failed to allocate RGB buffer\n");
        drm_gem_vunmap(gem_obj, vaddr);
        drm_dev_put(drm_dev);
        return -ENOMEM;
    }
    
    // Convertir el framebuffer a RGB888
    // El formato del framebuffer puede variar (XRGB8888, ARGB8888, etc.)
    // Aquí asumimos XRGB8888 (común en sistemas modernos)
    for (i = 0; i < capture_info_kernel.height; i++) {
        for (j = 0; j < capture_info_kernel.width; j++) {
            unsigned char *src = fb_data + (i * fb->pitches[0]) + (j * 4);
            unsigned char *dst = rgb_buffer + 
                               (i * capture_info_kernel.width * 3) + (j * 3);
            
            // XRGB8888 -> RGB888
            // Formato: [X][R][G][B] -> [R][G][B]
            dst[0] = src[2]; // R
            dst[1] = src[1]; // G
            dst[2] = src[0]; // B
        }
    }
    
    // Copiar los metadatos de vuelta al espacio de usuario
    if (copy_to_user(capture_info_user, &capture_info_kernel, 
                     sizeof(struct screen_capture_info))) {
        printk(KERN_ERR "screen_live: Failed to copy metadata to user\n");
        ret = -EFAULT;
        goto cleanup;
    }
    
    // Copiar el buffer RGB al espacio de usuario
    if (copy_to_user(capture_info_kernel.data, rgb_buffer, 
                     capture_info_kernel.buffer_size)) {
        printk(KERN_ERR "screen_live: Failed to copy buffer to user\n");
        ret = -EFAULT;
        goto cleanup;
    }
    
    printk(KERN_INFO "screen_live: Captured %ux%u screen (%lu bytes)\n",
           capture_info_kernel.width, capture_info_kernel.height,
           capture_info_kernel.buffer_size);

cleanup:
    kfree(rgb_buffer);
    drm_gem_vunmap(gem_obj, vaddr);
    drm_dev_put(drm_dev);
    
    return ret;
}