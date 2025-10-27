Forzar X11 a usar el Framebuffer Legacy

Vamos a hacer que tu servidor gráfico escriba en `/dev/fb0` para que tu syscall pueda capturarlo.

### Paso 1: Verificar qué servidor gráfico estás usando

```bash
echo $XDG_SESSION_TYPE
```

Si dice `x11`, perfecto. Si dice `wayland`, tendremos que cambiar a X11.

### Paso 2: Instalar el driver fbdev

```bash
sudo apt update
sudo apt install -y xserver-xorg-video-fbdev
```

### Paso 3: Configurar X11 para usar /dev/fb0

```bash
sudo mkdir -p /etc/X11/xorg.conf.d

sudo tee /etc/X11/xorg.conf.d/20-fbdev.conf > /dev/null <<'EOF'
Section "Device"
    Identifier  "VirtualBox Graphics"
    Driver      "fbdev"
    Option      "fbdev" "/dev/fb0"
    Option      "ShadowFB" "on"
EndSection

Section "Screen"
    Identifier  "Default Screen"
    Device      "VirtualBox Graphics"
EndSection
EOF
```

### Paso 4: Deshabilitar el driver DRM de VirtualBox (temporal)

Para forzar que use fbdev en lugar de DRM:

```bash
sudo tee /etc/modprobe.d/blacklist-vboxvideo.conf > /dev/null <<'EOF'
# Temporalmente deshabilitar vboxvideo DRM para forzar fbdev
blacklist vboxvideo
EOF
```

### Paso 5: Actualizar initramfs

```bash
sudo update-initramfs -u -k all
```

### Paso 6: Reiniciar

```bash
sudo reboot
```

---

## Después del reinicio - Verificar

Una vez reinicies (con tu kernel personalizado), verifica:

```bash
# 1. Verificar que estás en tu kernel
uname -r

# 2. Verificar que X11 está usando fbdev
cat /var/log/Xorg.0.log | grep fbdev

# 3. Debería mostrar algo como:
# [    XX.XXX] (II) LoadModule: "fbdev"
# [    XX.XXX] (II) Loading /usr/lib/xorg/modules/drivers/fbdev_drv.so

# 4. Verificar que /dev/fb0 está activo
ls -l /dev/fb0

# 5. Probar captura
cd ~/Documentos/sopes2/proyecto_sopes2_2S25/pruebas/screen
sudo ./test_screen
convert -size 1280x800 -depth 8 bgra:screenshot.raw screenshot.jpeg
```
