#include <stdio.h>
#include <stdlib.h>

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;

int main(int argc, char *argv[]) {

    if(argc != 5) {
        printf("Debe proporcionar 4 argumentos: [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de hilos]");
        // Ejemplo: ./filtro miImagen.png miImagen_output.png 15 4
        exit(0);
    }
    IMAGEN_ENTRADA = argv[1];
    IMAGEN_SALIDA = argv[2];
    ARG = atoi(argv[3]);
    NUM_HILOS = atoi(argv[4]);
    printf("%s %s %i %i\n", IMAGEN_ENTRADA, IMAGEN_SALIDA, ARG, NUM_HILOS);

    // TODO:

    return 0;
}