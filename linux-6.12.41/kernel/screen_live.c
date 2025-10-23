#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/file.h>
#include <linux/mm.h>

/* Declaraciones externas */
extern struct fb_info *registered_fb[FB_MAX];
extern int num_registered_fb;

struct screen_capture_info {
    __u32 width;
    __u32 height;
    __u32 bytes_per_pixel;
    __u64 buffer_size;
    void __user *data;
};

SYSCALL_DEFINE1(screen_live, struct screen_capture_info __user *, capture_info_user)
{
    struct screen_capture_info info_k;
    struct file *fb_filp = NULL;
    struct fb_info *fb_info_ptr;
    struct inode *inode;
    unsigned long fb_mem_size;
    void *kbuf = NULL;
    int ret = 0;
    int fbidx;

    if (!capture_info_user)
        return -EFAULT;

    if (copy_from_user(&info_k, capture_info_user, sizeof(info_k)))
        return -EFAULT;

    /* Abrir /dev/fb0 */
    fb_filp = filp_open("/dev/fb0", O_RDONLY, 0);
    if (IS_ERR(fb_filp)) {
        pr_err("screen_live: no se pudo abrir /dev/fb0\n");
        return PTR_ERR(fb_filp);
    }

    /* Obtener el fb_info desde el file */
    inode = file_inode(fb_filp);
    if (!inode) {
        pr_err("screen_live: no se pudo obtener inode\n");
        filp_close(fb_filp, NULL);
        return -EINVAL;
    }

    /* El minor number nos da el índice del framebuffer */
    fbidx = iminor(inode);
    
    /* Verificar que el framebuffer esté registrado */
    if (fbidx >= FB_MAX || !registered_fb[fbidx]) {
        pr_err("screen_live: framebuffer no registrado\n");
        filp_close(fb_filp, NULL);
        return -ENODEV;
    }

    fb_info_ptr = registered_fb[fbidx];

    /* Obtener dimensiones y formato */
    info_k.width = fb_info_ptr->var.xres;
    info_k.height = fb_info_ptr->var.yres;
    info_k.bytes_per_pixel = fb_info_ptr->var.bits_per_pixel / 8;
    
    /* Calcular tamaño del buffer */
    fb_mem_size = fb_info_ptr->fix.smem_len;
    if (fb_mem_size == 0) {
        fb_mem_size = (unsigned long)info_k.width * info_k.height * info_k.bytes_per_pixel;
    }
    info_k.buffer_size = fb_mem_size;

    /* Verificar que hay memoria mapeada */
    if (!fb_info_ptr->screen_base && !fb_info_ptr->screen_buffer) {
        pr_err("screen_live: no hay memoria de framebuffer disponible\n");
        filp_close(fb_filp, NULL);
        return -ENOMEM;
    }

    /* Alocar buffer temporal en kernel */
    kbuf = vmalloc(fb_mem_size);
    if (!kbuf) {
        pr_err("screen_live: no se pudo alocar buffer temporal\n");
        filp_close(fb_filp, NULL);
        return -ENOMEM;
    }

    /* Copiar datos del framebuffer al buffer temporal */
    if (fb_info_ptr->screen_base) {
        /* Memoria mapeada IO */
        memcpy_fromio(kbuf, fb_info_ptr->screen_base, fb_mem_size);
    } else if (fb_info_ptr->screen_buffer) {
        /* Memoria normal */
        memcpy(kbuf, fb_info_ptr->screen_buffer, fb_mem_size);
    }

    /* Copiar metadata al espacio de usuario */
    if (copy_to_user(capture_info_user, &info_k, sizeof(info_k))) {
        pr_err("screen_live: fallo al copiar metadata\n");
        ret = -EFAULT;
        goto cleanup;
    }

    /* Copiar datos de imagen al buffer de usuario */
    if (!info_k.data) {
        pr_warn("screen_live: puntero data es NULL\n");
        ret = -EFAULT;
        goto cleanup;
    }

    if (copy_to_user(info_k.data, kbuf, fb_mem_size)) {
        pr_err("screen_live: fallo al copiar datos de imagen\n");
        ret = -EFAULT;
        goto cleanup;
    }

    pr_info("screen_live: captura exitosa %ux%u bpp=%u (%llu bytes)\n",
            info_k.width, info_k.height, info_k.bytes_per_pixel, 
            (unsigned long long)fb_mem_size);

cleanup:
    vfree(kbuf);
    filp_close(fb_filp, NULL);
    return ret;
}