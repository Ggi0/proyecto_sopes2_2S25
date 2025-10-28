
## Documentación de Errores y Soluciones

### Problemas Detectados en las Syscalls del Mouse

Durante el desarrollo e integración de las **llamadas al sistema (syscalls)** encargadas de simular el movimiento y las acciones del mouse dentro del kernel de Linux, se detectaron varios problemas críticos que impedían el funcionamiento correcto de los módulos. A continuación, se documentan los principales errores encontrados, sus causas y las soluciones implementadas.

---

### **Error 1: Creación y destrucción del dispositivo en cada llamada**

#### **Descripción del problema**

En las primeras versiones de las syscalls `mouse_tracking` y `mouse_action`, el código creaba un dispositivo de entrada (`struct input_dev`) en cada invocación del sistema. Esto provocaba una sobrecarga significativa y una falta de estabilidad, ya que el kernel no disponía del tiempo suficiente para procesar los eventos antes de que el dispositivo fuera liberado.

Como consecuencia, los movimientos del cursor y los clicks no se reflejaban correctamente en el entorno gráfico. En algunos casos, se producían errores de registro o el sistema no reconocía los dispositivos virtuales creados.

#### **Causa técnica**

Cada ejecución de la syscall ejecutaba las funciones `input_allocate_device()` y `input_register_device()`, seguidas inmediatamente de `input_unregister_device()` y `input_free_device()`. Este ciclo continuo impedía mantener una sesión estable del dispositivo virtual y provocaba pérdidas de contexto en los controladores de entrada del sistema.

#### **Solución implementada**

Se modificó la arquitectura para **crear dispositivos persistentes globales**.
Cada dispositivo (`global_mouse_dev` y `global_click_dev`) se inicializa una sola vez mediante una función auxiliar (`init_mouse_device()` o `init_click_device()`) y se reutiliza en cada llamada posterior.

De esta forma, el dispositivo permanece registrado durante todo el tiempo de ejecución del módulo, garantizando que los eventos emitidos por las syscalls sean reconocidos por el sistema operativo.

```c
static struct input_dev *global_mouse_dev = NULL;

static int init_mouse_device(void)
{
    if (global_mouse_dev != NULL)
        return 0; // Ya inicializado
    ...
}
```

---

### **Error 2: Error de “double-free” en la syscall de clicks**

#### **Descripción del problema**

Durante la ejecución de la syscall `mouse_action`, el kernel arrojaba errores de **liberación doble de memoria (double-free)**. Esto ocurría cuando el código llamaba a `input_free_device()` después de que el dispositivo ya había sido liberado implícitamente por `input_unregister_device()`.

Este comportamiento generaba mensajes de advertencia en el log del kernel y, en algunos casos, provocaba inestabilidad en el sistema o errores de segmentación.

#### **Causa técnica**

La combinación incorrecta de funciones de liberación (`input_unregister_device()` y `input_free_device()`) causaba que la memoria asociada al dispositivo fuera liberada dos veces, una por cada llamada.

#### **Solución implementada**

Se eliminó la llamada redundante a `input_free_device()` después de `input_unregister_device()`.
Ahora, el flujo de registro y liberación del dispositivo es coherente y seguro, evitando la doble liberación de memoria.

```c
err = input_register_device(global_click_dev);
if (err) {
    pr_err("mouse_action: No se pudo registrar dispositivo\n");
    input_free_device(global_click_dev);
    global_click_dev = NULL;
    return -ENODEV;
}
```

---

### **Error 3: Dispositivo no reconocido por el entorno gráfico (X11/Wayland)**

#### **Descripción del problema**

A pesar de que los eventos del mouse se generaban correctamente, el entorno gráfico (X11 o Wayland) no reconocía los dispositivos virtuales creados, impidiendo que los movimientos o clicks se reflejaran en la pantalla.

#### **Causa técnica**

El identificador del bus del dispositivo (`bustype`) se configuraba como `BUS_VIRTUAL`, lo cual no era compatible con los controladores de entrada estándar utilizados por los entornos gráficos.

#### **Solución implementada**

Se cambió el valor del campo `bustype` a `BUS_USB`, simulando un dispositivo de mouse USB convencional.
De esta manera, los entornos gráficos reconocen correctamente al dispositivo virtual y procesan sus eventos como si provinieran de un mouse físico.

```c
global_mouse_dev->id.bustype = BUS_USB;
```

---

### **Error 4: Falta de sincronización entre hilos (race conditions)**

#### **Descripción del problema**

Durante pruebas simultáneas (por ejemplo, mover el mouse mientras se ejecutaba un click), se detectaban comportamientos inconsistentes e incluso bloqueos del sistema. Esto se debía a que las syscalls no controlaban el acceso concurrente a los dispositivos globales.

#### **Causa técnica**

Ambas syscalls (`mouse_tracking` y `mouse_action`) accedían a estructuras globales (`global_mouse_dev` y `global_click_dev`) sin mecanismos de sincronización, generando condiciones de carrera cuando eran invocadas de forma concurrente.

#### **Solución implementada**

Se incorporaron **mutexes** para garantizar exclusión mutua en las operaciones sobre los dispositivos globales.
Cada syscall bloquea el acceso mediante un `mutex_lock()` antes de operar sobre el dispositivo y lo libera al finalizar con `mutex_unlock()`.

```c
static DEFINE_MUTEX(mouse_lock);

mutex_lock(&mouse_lock);
/* operaciones críticas */
mutex_unlock(&mouse_lock);
```

Con esta modificación, las operaciones de movimiento y click son ahora **thread-safe** y pueden ejecutarse en paralelo sin riesgo de corrupción de memoria o inconsistencias.

---

### **Error 5: Dispositivo incompleto (falta de eventos habilitados)**

#### **Descripción del problema**

El sistema reconocía los dispositivos virtuales, pero estos no respondían correctamente a los eventos, especialmente en el caso del movimiento absoluto o clicks. El entorno gráfico los clasificaba como dispositivos parciales o sin capacidades completas de mouse.

#### **Causa técnica**

No se habían habilitado todos los tipos de eventos (`EV_ABS`, `EV_KEY`, `EV_REL`) ni los códigos de botones correspondientes (`BTN_LEFT`, `BTN_RIGHT`, `BTN_MIDDLE`), lo cual hacía que el dispositivo no fuera detectado como un mouse funcional completo.

#### **Solución implementada**

Se habilitaron todas las capacidades necesarias para un dispositivo de mouse completo.
Para el movimiento, se activaron eventos de tipo `EV_ABS` con los ejes `ABS_X` y `ABS_Y`, mientras que para las acciones de click se habilitaron `EV_KEY` junto con los tres botones principales.

```c
set_bit(EV_ABS, global_mouse_dev->evbit);
set_bit(ABS_X, global_mouse_dev->absbit);
set_bit(ABS_Y, global_mouse_dev->absbit);

set_bit(EV_KEY, global_mouse_dev->evbit);
set_bit(BTN_LEFT, global_mouse_dev->keybit);
set_bit(BTN_RIGHT, global_mouse_dev->keybit);
set_bit(BTN_MIDDLE, global_mouse_dev->keybit);
```

De esta manera, el entorno gráfico reconoce completamente los dispositivos y procesa los eventos correctamente.

---





### **Errores Detectados y Solución Implementada — Módulo de Captura de Teclado (Frontend → Backend → Syscall)**

---

#### **1. Descripción del Problema**

Durante la integración del flujo de captura de teclado entre el **frontend (React)**, el **backend (C++)** y la **syscall del kernel**, se detectó una discrepancia entre los **keycodes generados en el navegador (JavaScript)** y los **keycodes esperados por el subsistema input de Linux**.

El síntoma observado era que las teclas presionadas desde el navegador no correspondían a los caracteres simulados en el sistema remoto.

---

#### **2. Causa Raíz**

El origen del error radica en el uso de `event.keyCode` en el frontend, cuyo valor pertenece al **DOM KeyboardEvent estándar de JavaScript**, mientras que la syscall requiere los **Linux Input Keycodes** definidos en `linux/input-event-codes.h`.

**Ejemplo de discrepancia:**

| Tecla | JS keyCode | Linux keycode (input-event) |
| :---- | :--------: | :-------------------------: |
| a     |     65     |              30             |
| b     |     66     |              48             |
| c     |     67     |              46             |
| 1     |     49     |              2              |
| 2     |     50     |              3              |
| Enter |     13     |              28             |
| Space |     32     |              57             |

Como resultado, la syscall recibía valores inválidos o incorrectos, impidiendo la simulación correcta del teclado en el sistema remoto.

---

#### **3. Solución Aplicada**

Se decidió **trasladar la lógica de conversión de códigos al backend**, aprovechando que ya existía una función de mapeo (`charToKeycode()`) implementada en C++.

De esta forma, el **frontend** envía únicamente el **carácter presionado**, y el **backend** traduce este carácter al keycode Linux correspondiente.

---

#### **4. Cambios Realizados**

##### **Frontend (`RemoteDesktop.jsx`)**

Se modificó el método `handleKeyDown` para enviar el carácter (`event.key`) en lugar del código numérico (`event.keyCode`).

**Antes:**

```js
const keycode = event.keyCode || event.which;
await apiService.keyPress(keycode, token);
```

**Después:**

```js
const handleKeyDown = async (event) => {
  if (!canControl()) return;

  event.preventDefault();
  const key = event.key;

  // Filtra teclas no relevantes (Shift, Alt, etc.)
  if (key.length !== 1 && key !== 'Enter' && key !== 'Backspace' && key !== ' ') return;

  try {
    await apiService.keyPress(key, token);
  } catch (error) {
    console.error('Error al enviar tecla:', error);
  }
};
```

---

##### **Frontend (`services/apiService.js`)**

Se actualizó la función `keyPress` para enviar el carácter en formato JSON.

**Antes:**

```js
keyPress: async (keycode, token) => {
  return request('/api/keyboard/press', {
    method: 'POST',
    headers: { 'Authorization': `Bearer ${token}` },
    body: JSON.stringify({ keycode }),
  });
},
```

**Después:**

```js
keyPress: async (key, token) => {
  return request('/api/keyboard/press', {
    method: 'POST',
    headers: { 'Authorization': `Bearer ${token}` },
    body: JSON.stringify({ key }),
  });
},
```

---

##### **Backend (C++)**

El backend ya incluía soporte para manejar la conversión de caracteres a keycodes mediante la función `charToKeycode()`:

```cpp
if (json_data.has("keycode")) {
    // Manejo directo de keycode
} else if (json_data.has("key")) {
    std::string key_str = json_data["key"].s();
    if (key_str.length() == 1) {
        keycode = Syscalls::charToKeycode(key_str[0]);
    }
}
```

De esta manera, el backend convierte el carácter en su código Linux antes de invocar la syscall.

---

#### **5. Resultado Final**

1. El navegador detecta la tecla presionada (por ejemplo, `'a'`).
2. El frontend envía la petición JSON `{"key": "a"}` al backend.
3. El backend traduce `'a'` a su keycode Linux `30` usando `charToKeycode()`.
4. Se invoca la syscall con el keycode correcto (`30`), que emula la tecla `'a'` correctamente en el sistema remoto.

---

#### **6. Fuente de Referencia**

La tabla completa de equivalencias entre **Linux Input Keycodes** y **valores hexadecimales** se obtuvo de la siguiente fuente:

**Repositorio:**
[https://gist.github.com/rickyzhang82/8581a762c9f9fc6ddb8390872552c250](https://gist.github.com/rickyzhang82/8581a762c9f9fc6ddb8390872552c250)

Este documento contiene el mapeo oficial de teclas definido en `linux/input-event-codes.h`, utilizado como referencia para validar las conversiones en `charToKeycode()`.

---


### **Error Detectado y Solución — Captura de Pantalla con Framebuffer (/dev/fb0)**

---

#### **1. Descripción del Problema**

Durante la ejecución de la syscall encargada de capturar la pantalla, se observó que las imágenes obtenidas correspondían únicamente al **framebuffer estático** del arranque del sistema (boot), y no al contenido dinámico del escritorio o entorno gráfico activo (Cinnamon/X11).

Esto indicaba que el sistema gráfico no estaba escribiendo en `/dev/fb0`, sino utilizando directamente el **subsistema DRM/KMS** (Direct Rendering Manager / Kernel Mode Setting), el cual gestiona la representación de gráficos en memoria sin pasar por el framebuffer tradicional.

---

#### **2. Causa Raíz**

Los entornos de escritorio modernos (como **Cinnamon**, **GNOME**, o **KDE Plasma**) utilizan **compositores gráficos basados en DRM**, lo que significa que el contenido visual de las ventanas se renderiza directamente sobre el buffer de video del dispositivo mediante controladores como `vboxvideo`, `amdgpu`, `intel`, o `nvidia`.

En consecuencia:

* El archivo `/dev/fb0` no refleja el estado actual de la pantalla.
* La syscall que intenta leer de `/dev/fb0` obtiene una imagen vacía o congelada en el estado inicial del arranque.

---

#### **3. Objetivo de la Solución**

Reconfigurar el servidor gráfico **X11** para utilizar el **driver framebuffer (fbdev)**, de modo que todo el contenido gráfico sea renderizado nuevamente a través de `/dev/fb0`, permitiendo que la syscall capture la pantalla en tiempo real.

---

#### **4. Solución Aplicada**

##### **Paso 1: Verificar el tipo de sesión gráfica**

Determinar si el sistema está utilizando **X11** o **Wayland**:

```bash
echo $XDG_SESSION_TYPE
```

* Si la salida es `x11`, se puede continuar.
* Si la salida es `wayland`, es necesario iniciar sesión con **X11** (seleccionarlo en la pantalla de inicio de sesión).

---

##### **Paso 2: Instalar el controlador framebuffer (fbdev)**

Instalar el paquete del controlador X11 que permite el uso del framebuffer legacy:

```bash
sudo apt update
sudo apt install -y xserver-xorg-video-fbdev
```

---

##### **Paso 3: Configurar X11 para usar `/dev/fb0`**

Crear la configuración de Xorg para forzar el uso del driver `fbdev`:

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

Esta configuración instruye a X11 para redirigir toda la salida gráfica al framebuffer `/dev/fb0`.

---

##### **Paso 4: Deshabilitar el driver DRM (vboxvideo)**

El controlador `vboxvideo` de VirtualBox usa DRM y evita que X11 escriba sobre `/dev/fb0`.
Para forzar el uso exclusivo de fbdev, se desactiva temporalmente este módulo:

```bash
sudo tee /etc/modprobe.d/blacklist-vboxvideo.conf > /dev/null <<'EOF'
# Deshabilitar vboxvideo DRM para usar fbdev
blacklist vboxvideo
EOF
```

---

##### **Paso 5: Actualizar la imagen del kernel (initramfs)**

Actualizar el initramfs para aplicar la configuración del sistema:

```bash
sudo update-initramfs -u -k all
```

---

##### **Paso 6: Reiniciar el sistema**

Aplicar todos los cambios:

```bash
sudo reboot
```

---

#### **5. Verificación Posterior al Reinicio**

Una vez reiniciado el sistema:

1. **Verificar el kernel activo:**

   ```bash
   uname -r
   ```

2. **Confirmar que X11 está usando fbdev:**

   ```bash
   cat /var/log/Xorg.0.log | grep fbdev
   ```

   La salida debe incluir líneas como:

   ```
   (II) LoadModule: "fbdev"
   (II) Loading /usr/lib/xorg/modules/drivers/fbdev_drv.so
   ```

3. **Comprobar que `/dev/fb0` está disponible:**

   ```bash
   ls -l /dev/fb0
   ```

4. **Probar la captura con la syscall:**

   ```bash
   cd ~/Documentos/sopes2/proyecto_sopes2_2S25/pruebas/screen
   sudo ./test_screen
   ```

5. **Convertir la imagen capturada para verificar visualmente:**

   ```bash
   convert -size 1280x800 -depth 8 bgra:screenshot.raw screenshot.jpeg
   ```

---

#### **6. Resultado Final**

Tras aplicar la configuración anterior:

* El servidor gráfico X11 escribe nuevamente en `/dev/fb0`.
* La syscall puede leer correctamente los datos de framebuffer.
* Las capturas de pantalla reflejan el estado actual del escritorio y las ventanas activas.

De esta manera se logró que la syscall recupere **frames dinámicos del entorno gráfico** en tiempo real.

---
