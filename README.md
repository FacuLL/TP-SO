TP 1 - SISTEMAS OPERATIVOS 2026

GRUPO 2

Antonio Valentinuzzi
Facundo López Llopis
Tobias Young

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

- Inicializa los argumentos:
  * Separamos la logica de argumentos en otro archivo pues en un momento se quiso reutilizar para los demas procesos, pero no fue posible. De todas maneras, mejora la legibilidad del master.
  * Usamos getopt porque es mucho mas facil que parsear a mano.
  * Creamos un struct aparte que contenga los argumentos. Esto permite tener mas organizado y librarnos de las variables globales separadas para seed, view_path, etc.
  * En un principio se penso en guardar los argumentos directo en el struct del estado del juego. Esto implicaba incializar memoria compartida, llenarla con los argumentos y luego resizear para contemplar el tablero de tamaño dinamico. Este método era imposible ya que resizear implicaba cerrar y volver a abrir la memoria compartida, destruyendo los argumentos.
- Inicializa las memorias compartidas para el estado del juego y para los semaforos:
  * Separamos en shared.c pues asi mantenemos la logica de memorias compartidas en un archivo que importan todos.
  * Cerramos los fd directamente en la misma funcion, luego de mapear, para no tener que preocuparse luego por ello (Ademas, una vez mapeado ya no sirve de nada el fd).
- Crea un proceso hijo por cada jugador utilizando fork:
  * Se crea un pipe por cada player. El hijo (player) escribe en el pipe y el padre (master) lee del pipe.
  * El padre rellena el struct del estado con los pid de los hijos.
  * Los hijos cierran TODOS los fd luego de clonar el suyo como STDOUT.
  * Antes de ejecutar el programa del player, espera a que el padre haya escrito su pid en el struct, con su semaforo correspondiente.
- Inicializa la vista del sistema con fork. Cierra todos los fd.
- Empieza el ciclo del juego, hasta el game over:
  * Con select se espera a la escritura de cualquier fd. Solo agregamos los fd de aquellos que no estan bloqueados.
  * Aprovechamos la funcionalidad de que el select altera el timeout con el tiempo restante, para que la estructura del timeout se restablezca unicamente cuando hay un movimiento válido.
  * Los unicos momentos del ciclo donde el master no tiene acceso al game state es durante el select y durante el delay. Inicialmente solo se entraba a la zona de escritura cuando habia un movimiento valido o cuando habia un movimiento invalido, pero esto implicaba agregar muchisima logica de semaforos que hacia engorroso leer el master. Finalmente se decidió hacer el master entre luego del select y, en caso de ser necesario, salga durante el delay, ingresando nuevamente al finalizar el mismo.
  * Se valida el movimiento y modifica al estado del juego en consecuencia.
  * Si el jugador termina, se bloquea
  * El juego termina cuando todos se bloquean.
- Se espera a que la vista y los players terminen (por las dudas se deja a los jugadores jugar).
- Se imprimen los resultados y se limpia memoria compartida, mallocs, semaforos y fds.
- En caso de errores, tambien se limpia todo lo que ya fue inicializado.


PLAYER

- Se conecta a las memorias compartidas con la libreria previamente mencionada.
- Espera a que su pid este disponible en el struct del estado del juego.
- Recorre los players hasta encontrar su pid, para conocer su índice.
- Empieza el ciclo:
  * En cada ciclo espera a poder moverse.
  * Accede al estado del juego con los semaforos, aplicando el patron light switch y evitando la inanición del master.
  * Calcula su movimiento y lo imprime en salida estandar (pipe).
- Al finalizar, limpia el pipe y las memorias compartidas.

VISTA

- Se utiliza ncurses como interfaz gráfica en terminal.
- La instalación de la librería está incluida en el Makefile.
- Se conecta a las memorias compartidas y espera con el semaforo al contenido para imprimir.
-
- Al terminar, limpia las memorias compartidas.


--------------------------------------------------

ESTRUCTURA DEL PROYECTO

.
├── src/
├── include/
├── Makefile
└── README


--------------------------------------------------

RUTAS RELATIVAS

Estas rutas se generan dentro del repositorio.

./view
./master
./player

--------------------------------------------------

Limitaciónes

En caso de que se ponga un player que no se elimine por su cuenta, como se requiere la catedra, el programa no finalizara y se quedara esperando

--------------------------------------------------

FIN