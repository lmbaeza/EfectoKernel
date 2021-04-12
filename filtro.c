#include <stdio.h>
#include <stdlib.h>

#include "sod/sod.h"

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;

int main(int argc, char *argv[]) {

    if(argc != 5) {
        printf("Debe proporcionar 4 argumentos: [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de hilos]");
        // Ejemplo: ./filtro.o input.png output.png 15 4
        exit(0);
    }
    IMAGEN_ENTRADA = argv[1];
    IMAGEN_SALIDA = argv[2];
    ARG = atoi(argv[3]);
    NUM_HILOS = atoi(argv[4]);
    printf("%s %s %i %i\n", IMAGEN_ENTRADA, IMAGEN_SALIDA, ARG, NUM_HILOS);

    // TODO:

    sod_img imgIn = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);

    if (imgIn.data == 0) {
		// Validar que la imagen exista
		printf("No pudo cargar la imagen %s\n", IMAGEN_ENTRADA);
		return 0;
	}

    // Modificar la imagen
    for(int i = 0; i  < 100; ++i) {
        for(int j = 0; j < 100; ++j) {
            sod_img_set_pixel(imgIn, i, i+j, 0, 0.3434);
        }
    }

	sod_img_save_as_png(imgIn, IMAGEN_SALIDA);

    // Liberar la Memoria
	sod_free_image(imgIn);

    return 0;
}