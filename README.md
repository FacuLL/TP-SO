TP 1 - SISTEMAS OPERATIVOS 2026

GRUPO 2

Antonio Valentinuzzi


--------------------------------------------------

ENTORNO DE EJECUCIÓN

El proyecto está pensado para ejecutarse dentro del contenedor Docker provisto por la cátedra.

1. Descargar la imagen:

docker pull agodio/itba-so-multiarch:3.1

2. Ejecutar el contenedor:

docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multiarch:3.1

--------------------------------------------------

COMPILACIÓN

Dentro del contenedor ejecutar:

cd /root
make all

Esto compilara los programas de player master y view respectivamente.

--------------------------------------------------

LIMPIEZA 

Dentro del contenedor ejecutar:

Make clean

--------------------------------------------------

TEST PVS

Dentro de contenedor ejecutar: 

cd ./root 

Make clean

Make test

Peden eliminarse los archivos con el comando:

Make test-clean

--------------------------------------------------

EJECUCIÓN

./master [opciones]

Ejemplo:

./master -p ./player -view -w 10 -h 10 

Parámetros (ejemplo):
-w : ancho del tablero
-h : alto del tablero
-p : dirección del jugador
-v : dirección de la view
-d : delay en ms
-t : timeout en s
-s : seed

--------------------------------------------------

DECISIONES DE DISEÑO

MASTER

- Inicializa las memorias compartidas.
- Crea un proceso hijo por cada jugador utilizando fork().
- Se crea un pipe por cada hijo:
  * El hijo escribe en el pipe
  * El padre lee del pipe
- Los pipes se almacenan en un arreglo indexado por el id del jugador.
- Inicializa la vista del sistema.

PLAYER

- Cada jugador corre en un proceso independiente.
- Calcula sus movimientos en base al estado compartido.
- Valida movimientos para no salir del tablero ni colisionar.
- Se comunica con el master mediante pipes.

VISTA

- Se utiliza ncurses como interfaz gráfica en terminal.
- Fue elegida por su simplicidad y recomendación de la cátedra.
- La instalación de la librería está incluida en el Makefile.

--------------------------------------------------

ESTRUCTURA DEL PROYECTO

.
├── src/
├── include/
├── Makefile
└── README


--------------------------------------------------

RUTAS RELATIVAS

Estas rutas se generan dentro del repositorio

./view
./master
./player

--------------------------------------------------

Limitaciónes

En caso de que se ponga un player que no se elimine por su cuenta, como se requiere en la catedra. 
El programa no finalizara y se quedara esperando

--------------------------------------------------

FIN