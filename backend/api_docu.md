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