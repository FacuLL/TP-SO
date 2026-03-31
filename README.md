# TP 1 SISTEMAS OPERATIVOS 2026

## GRUPO 2

## Compilación

### Descargar e Inicializar el docker provisto por la catedra

    docker pull agodio/itba-so-multiarch:3.1

    docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multiarch:3.1 

### Compilar el projecto

    cd root
    make all

## Decisiones de diseño.

### Master

    Se inicializan las memorias compartidas.
    Se inicializa un pipe por cada hijo. En el cual, el hijo escribe y el padre lee. 

### Player



### Vista

# EOF