

en la carpeta proyecto_sopes2_2s25 ejecutar:

`npm create vite@lastest`

pasos:

```bash
Project name → escribe: frontend

Framework → selecciona: React

Variant → selecciona: JavaScript

Use rolldown-vite (Experimental)? → No

Install with npm and start now? → Yes
```

en la carpeta frontend ejecutar: 
`npm run dev` --> para levantar el front

-------------------------------------------------------

consideraciones: instalar node y npm para el funcionamiento correcto:
```bash
# Instala nvm si no lo tienes
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
source ~/.bashrc

# Instala Node.js 20
nvm install 20
nvm use 20
```



si clono el repositorio, los node_modules no estaran instalados, dirigirse a frontend y ejecutar:
```bash
npm install
```

posteriormente pordra levantar el frontend de forma convencional:
```bash
npm run dev
```