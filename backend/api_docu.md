Documentacion backend 

utilizando crow

instalar dependencias:
sudo apt update

sudo apt install cmake libboost-all-dev


cloanr CROW y copiar el header unico
git clone https://github.com/CrowCpp/Crow.git


# Desde la carpeta backend/
mkdir -p build
cd build
cmake ..
make

# Ejecutar
./servidor


 ----------------------------------------------------------------
para PAM:
Configuración del Sistema Linux

### 1. Instalar dependencias de PAM

```bash
sudo apt-get update
sudo apt-get install -y libpam0g-dev
```

### 2. Crear grupos de acceso

```bash
# Crear grupos
sudo groupadd remote_view
sudo groupadd remote_control

# Verificar que se crearon
getent group remote_view
getent group remote_control
```

### 3. Crear usuario admin con contraseña "123"

```bash
# Crear usuario
sudo useradd -m -s /bin/bash admin

# Establecer contraseña "---"
echo "admin:---" | sudo chpasswd

# Agregar al grupo remote_control
sudo usermod -aG remote_control admin

# Verificar
id admin
# Debe mostrar: groups=1001(admin),1002(remote_control)
```

### 4. Crear usuarios de prueba (opcional)

```bash
# Usuario con solo visualización
sudo useradd -m -s /bin/bash viewer
echo "viewer:viewer123" | sudo chpasswd
sudo usermod -aG remote_view viewer

# Usuario con control completo
sudo useradd -m -s /bin/bash controller
echo "controller:controller123" | sudo chpasswd
sudo usermod -aG remote_control controller

# Tu usuario actual
sudo usermod -aG remote_control vgio
```

### 5. Configurar permisos para PAM

El ejecutable necesita permisos especiales para usar PAM:

```bash
# Opción 1: Ejecutar con sudo (desarrollo)
sudo ./servidor

# Opción 2: Setear capabilities (producción)
sudo setcap cap_dac_read_search+ep ./servidor
```

---

## PARTE 4: Compilación y Ejecución

```bash
cd ~/Documentos/sopes2/proyecto_sopes2_2S25/backend

# Limpiar build anterior
rm -rf build

# Compilar
mkdir build
cd build
cmake ..
make -j$(nproc)

# Ejecutar (con sudo para PAM)
sudo ./servidor
```






