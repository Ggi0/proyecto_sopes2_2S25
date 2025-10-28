#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/fb.h>

struct screen_capture_info {
    __u32 width;
    __u32 height;
    __u32 bytes_per_pixel;
    __u64 buffer_size;
    void __user *data;
};

/* Declarar símbolos externos del framebuffer */
extern struct fb_info *registered_fb[];
extern int num_registered_fb;

SYSCALL_DEFINE1(screen_live, struct screen_capture_info __user *, capture_info_user)
{
    struct screen_capture_info info_k;
    struct fb_info *fb_info_ptr;
    void *kbuf = NULL;
    unsigned long fb_size;
    int ret = 0;

    if (!capture_info_user)
        return -EFAULT;

    if (copy_from_user(&info_k, capture_info_user, sizeof(info_k)))
        return -EFAULT;

    /* Verificar que hay un framebuffer registrado */
    if (num_registered_fb < 1 || !registered_fb[0]) {
        pr_err("screen_live: no hay framebuffer registrado\n");
        return -ENODEV;
    }

    fb_info_ptr = registered_fb[0];

    /* Forzar sincronización del framebuffer si el driver lo soporta */
    if (fb_info_ptr->fbops) {
        /* Intentar sync */
        if (fb_info_ptr->fbops->fb_sync)
            fb_info_ptr->fbops->fb_sync(fb_info_ptr);
        
        /* Intentar pan_display para forzar actualización */
        if (fb_info_ptr->fbops->fb_pan_display) {
            struct fb_var_screeninfo var = fb_info_ptr->var;
            fb_info_ptr->fbops->fb_pan_display(&var, fb_info_ptr);
        }
    }

    /* Obtener dimensiones */
    info_k.width = fb_info_ptr->var.xres;
    info_k.height = fb_info_ptr->var.yres;
    info_k.bytes_per_pixel = fb_info_ptr->var.bits_per_pixel / 8;
    
    fb_size = fb_info_ptr->fix.smem_len;
    if (fb_size == 0)
        fb_size = info_k.width * info_k.height * info_k.bytes_per_pixel;
    
    info_k.buffer_size = fb_size;

    /* Verificar memoria disponible */
    if (!fb_info_ptr->screen_base && !fb_info_ptr->screen_buffer) {
        pr_err("screen_live: no hay memoria de framebuffer\n");
        return -ENOMEM;
    }

    /* Alocar buffer temporal */
    kbuf = vmalloc(fb_size);
    if (!kbuf) {
        pr_err("screen_live: no se pudo alocar buffer temporal\n");
        return -ENOMEM;
    }

    /* Copiar datos del framebuffer */
    if (fb_info_ptr->screen_base) {
        memcpy_fromio(kbuf, fb_info_ptr->screen_base, fb_size);
    } else if (fb_info_ptr->screen_buffer) {
        memcpy(kbuf, fb_info_ptr->screen_buffer, fb_size);
    }

    /* Copiar metadata al usuario */
    if (copy_to_user(capture_info_user, &info_k, sizeof(info_k))) {
        ret = -EFAULT;
        goto cleanup;
    }

    /* Copiar datos al usuario */
    if (!info_k.data) {
        ret = -EFAULT;
        goto cleanup;
    }

    if (copy_to_user(info_k.data, kbuf, fb_size)) {
        ret = -EFAULT;
        goto cleanup;
    }

    pr_info("screen_live: captura %ux%u bpp=%u (%lu bytes)\n",
            info_k.width, info_k.height, info_k.bytes_per_pixel, fb_size);

cleanup:
    vfree(kbuf);
    return ret;
}