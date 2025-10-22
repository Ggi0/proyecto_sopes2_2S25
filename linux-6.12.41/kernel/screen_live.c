#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/errno.h>
#include <linux/types.h>

struct screen_capture_info {
    __u32 width;
    __u32 height;
    __u32 bytes_per_pixel;    /* en este ejemplo asumimos 4 (RGBA/ARGB/XRGB) por defecto */
    __u64 buffer_size;
    void __user *data;        /* puntero en espacio de usuario al buffer donde se escribirá */
};

/*
 * SYSCALL: screen_live
 *
 * - Copia desde /sys/class/drm/.../modes la primera línea para obtener la resolución.
 * - Intenta abrir /dev/fb0 y leer el framebuffer crudo hasta buffer_size bytes.
 * - Copia metadata de vuelta a la estructura de usuario y copia el buffer de pixeles al puntero user->data.
 *
 * Nota: esto lee datos *crudos* de /dev/fb0; su formato depende del driver (puede ser XRGB, ARGB, BGR, etc.).
 * El espacio de usuario deberá interpretar/convertir según corresponda (p. ej. suponer XRGB8888).
 *
 * Este enfoque evita usar símbolos DRM internos que no siempre están exportados para su uso desde syscall.
 */
SYSCALL_DEFINE1(screen_live, struct screen_capture_info __user *, capture_info_user)
{
    struct screen_capture_info info_k;
    struct file *f_modes = NULL;
    struct file *f_fb = NULL;
    char modes_buf[128];
    ssize_t n;
    loff_t pos;
    __u32 w = 0, h = 0;
    __u32 bpp = 4; /* por defecto asumimos 4 bytes/pixel (32bpp) */
    void *kbuf = NULL;
    int ret = 0;

    if (!capture_info_user)
        return -EFAULT;

    /* copiar la estructura (la parte que el usuario haya rellenado; sobreescribiremos campos) */
    if (copy_from_user(&info_k, capture_info_user, sizeof(info_k)))
        return -EFAULT;

    /* 1) leer resolución desde sysfs: (ruta típica; adapta si tu conector tiene otro nombre) */
    f_modes = filp_open("/sys/class/drm/card0-Virtual-1/modes", O_RDONLY, 0);
    if (IS_ERR(f_modes)) {
        /* si no existe, intentamos la ruta genérica card0/modes como fallback */
        f_modes = filp_open("/sys/class/drm/card0/modes", O_RDONLY, 0);
        if (IS_ERR(f_modes)) {
            pr_warn("screen_live: no pude abrir sysfs drm modes (/sys/class/drm/card0-Virtual-1/modes ni /sys/class/drm/card0/modes)\n");
            ret = -ENODEV;
            goto out;
        }
    }

    /* leer la primera línea */
    pos = 0;
    n = kernel_read(f_modes, modes_buf, sizeof(modes_buf) - 1, &pos);

    if (n <= 0) {
        pr_warn("screen_live: lectura modes sysfs vacía o error\n");
        ret = -EIO;
        goto close_modes;
    }
    modes_buf[n] = '\0';

    /* parsear "1406x738" (formato esperado: %ux%u) */
    if (sscanf(modes_buf, "%u x %u", &w, &h) != 2) {
        if (sscanf(modes_buf, "%ux%u", &w, &h) != 2) {
            pr_warn("screen_live: formato modes inesperado: '%s'\n", modes_buf);
            ret = -EINVAL;
            goto close_modes;
        }
    }

    /* no quemamos la resolución en tu struct en userspace si el usuario ya puso algo,
       pero sobreescribimos con la detectada (tal como pediste, no "quemar" para front: aquí la devolvemos). */
    info_k.width = w;
    info_k.height = h;
    info_k.bytes_per_pixel = bpp;
    info_k.buffer_size = (u64)w * (u64)h * (u64)bpp;

    /* 2) reservar buffer kernel temporal */
    kbuf = kmalloc(info_k.buffer_size, GFP_KERNEL);
    if (!kbuf) {
        ret = -ENOMEM;
        goto close_modes;
    }
    memset(kbuf, 0, info_k.buffer_size);

    /* 3) intentar abrir /dev/fb0 y leer su contenido en kbuf */
    f_fb = filp_open("/dev/fb0", O_RDONLY, 0);
    if (IS_ERR(f_fb)) {
        pr_warn("screen_live: no se pudo abrir /dev/fb0 (driver fb absent o no exportado). errno=%ld\n", PTR_ERR(f_fb));
        ret = -ENODEV;
        goto free_kbuf;
    }

    /* leer hasta buffer_size bytes (lectura cruda; algunos drivers fb soportan lectura directa) */
    pos = 0;
    n = kernel_read(f_fb, kbuf, (size_t)info_k.buffer_size, &pos);


    if (n < 0) {
        pr_warn("screen_live: error leyendo /dev/fb0: %zd\n", n);
        ret = (int)n;
        goto close_fb;
    }
    /* si la lectura devolvió menos bytes, ajustamos buffer_size para no copiar basura */
    if ((size_t)n < info_k.buffer_size)
        info_k.buffer_size = n;

    /* 4) copiar metadata actualizada a espacio de usuario */
    if (copy_to_user(capture_info_user, &info_k, sizeof(info_k))) {
        ret = -EFAULT;
        goto close_fb;
    }

    /* 5) copiar los bytes crudos leídos al buffer de usuario indicado por capture_info_user->data */
    if (!info_k.data) {
        pr_warn("screen_live: puntero data en user struct es NULL\n");
        ret = -EFAULT;
        goto close_fb;
    }

    if (copy_to_user(info_k.data, kbuf, (size_t)info_k.buffer_size)) {
        pr_warn("screen_live: fallo copy_to_user para buffer de pixeles\n");
        ret = -EFAULT;
        goto close_fb;
    }

    pr_info("screen_live: leído %llu bytes de /dev/fb0 (res %ux%u, bpp=%u)\n",
            (unsigned long long)info_k.buffer_size, info_k.width, info_k.height, info_k.bytes_per_pixel);

close_fb:
    filp_close(f_fb, NULL);
free_kbuf:
    kfree(kbuf);
close_modes:
    filp_close(f_modes, NULL);
out:
    return ret;
}
