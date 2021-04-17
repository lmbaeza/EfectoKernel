#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sod/sod.h"

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;

float kernel[3][3] = {{-1,-1,-1}, 
                    {-1, 8,-1},
                    {-1,-1,-1}};

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
    sod_img imgOut = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);

    if (imgIn.data == 0) {
        // Validar que la imagen exista
        printf("No pudo cargar la imagen %s\n", IMAGEN_ENTRADA);
        return 0;
    }

    // Modificar la imagen
    const int TAM_COLORES = 9;
    float colores[9];

    printf("Tamaño de la Imange %dx%d\n", imgIn.w, imgIn.h);

    for(int y = 1; y < imgIn.h-1; ++y) {
        for(int x = 1; x < imgIn.w-1; ++x) {
            float sum = 0.0;
            for(int ky = -1; ky <= 1; ++ky){
                for(int kx = -1; kx <= 1; ++kx){
                    float val = sod_img_get_pixel(imgIn, x+kx, y+ky, 0);
                    sum += kernel[ky+1][kx+1] * val;
                }
            }
            sod_img_set_pixel(imgOut, x, y, 0, abs(sum));            
            sod_img_set_pixel(imgOut, x, y, 1, abs(sum));            
            sod_img_set_pixel(imgOut, x, y, 2, abs(sum));            
        }
    }

    sod_img_save_as_png(imgOut, IMAGEN_SALIDA);

    // Liberar la Memoria
    sod_free_image(imgIn);
    sod_free_image(imgOut);

    return 0;
}