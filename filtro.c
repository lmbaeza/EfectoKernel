#include <stdio.h>
#include <stdlib.h>

#include "sod/sod.h"

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;

#define eps  0.000000001
int compare(const void *_a, const void *_b) {
    float *a, *b;
    a = (float*) _a;
    b = (float*) _b;
    return ( (((*a) + eps) < (*b))? -1 :( (((*b) + eps) < (*a) )? 1 : 0) );
}

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
    const int TAM_COLORES = 9;
    float colores[9];

    for(int i = 1; i+1 < imgIn.w; ++i) {
        for(int j = 1; j+1 < imgIn.h; ++j) {
            // float sod_img_get_pixel(sod_img input, int x, int y, int c);
            colores[0] = sod_img_get_pixel(imgIn, i-1, j-1, 0);
            colores[1] = sod_img_get_pixel(imgIn, i-1, j, 0);
            colores[2] = sod_img_get_pixel(imgIn, i-1, j + 1, 0);
            colores[3] = sod_img_get_pixel(imgIn, i, j-1, 0);
            colores[4] = sod_img_get_pixel(imgIn, i, j, 0);
            colores[5] = sod_img_get_pixel(imgIn, i, j + 1, 0);
            colores[6] = sod_img_get_pixel(imgIn, i + 1, j-1, 0);
            colores[7] = sod_img_get_pixel(imgIn, i + 1, j, 0);
            colores[8] = sod_img_get_pixel(imgIn, i + 1, j + 1, 0);

            qsort(colores, TAM_COLORES, sizeof(float), &compare);
            int idx = 4;
            sod_img_set_pixel(imgIn, i, j, 0,colores[idx]);
        }
    }

    sod_img_save_as_png(imgIn, IMAGEN_SALIDA);

    // Liberar la Memoria
    sod_free_image(imgIn);

    return 0;
}