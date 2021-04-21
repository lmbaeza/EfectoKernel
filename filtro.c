#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "sod/sod.h"

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;

float kernel[3][3] = {{-1,-1,-1}, 
                      {-1, 8,-1},
                      {-1,-1,-1}};

int intervalo[16][2]; // El i-th hilo vá desde el intervalo intervalo[i][0] hasta intervalo[i][1]
 
sod_img imgIn;
sod_img imgOut;

void * filter(void * arg) {
    // Id del Hilo
    int threadId = *(int*) arg;
    // Obtener intervalo, desde-hasta
    int from = intervalo[threadId][0];
    int to = intervalo[threadId][1];

    for(int y = from; y <= to; ++y) {
        for(int x = 1; x < imgIn.w-1; ++x) {

            float sum = 0.0;
            // Iterar la matrix 3x3 kernel
            for(int ky = -1; ky <= 1; ++ky){
                for(int kx = -1; kx <= 1; ++kx){
                    // Obtener pixel (Red) en la coordenada (x+kx, y+ky)
                    float val = sod_img_get_pixel(imgIn, x+kx, y+ky, 0); // R
                    sum += kernel[ky+1][kx+1] * val;

                    //  kernel       pixeles de la imagen, px=Pixel

                    // [k1 k2 k3]    [px1 px2 px3]
                    // [k4 k5 k6]    [px4 px5 px6]
                    // [k7 k8 k9]    [px7 px8 px9]

                    // sum = k1*px1 + k2*px2 + ... + k9*px9
                }
            }

            // Cambiar el pixel (Red) de la coordenada (x, y)
            sod_img_set_pixel(imgOut, x, y, 0, abs(sum)); // R

            // Cambiar el pixel (Green) de la coordenada (x, y)
            sod_img_set_pixel(imgOut, x, y, 1, abs(sum)); // G

            // Cambiar el pixel (Blue) de la coordenada (x, y)
            sod_img_set_pixel(imgOut, x, y, 2, abs(sum)); // B
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 5) {
        printf("Debe proporcionar 4 argumentos: [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de hilos]");
        // Ejemplo: ./filtro.o img/input1.png img/output1.png 8 16
        exit(0);
    }
    // Ruta de la imagen de entrada: Ej: img/input1.png
    IMAGEN_ENTRADA = argv[1];

    // Ruta de la imagen de salida: Ej: img/output1.png
    IMAGEN_SALIDA = argv[2];

    // Argumento del filtro: Ej 8
    ARG = atof(argv[3]);
    kernel[1][1] = ARG;

    // Numero de hilos utilizados
    NUM_HILOS = atoi(argv[4]);

    // Cargar Imagen en memoria
    imgIn = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);
    imgOut = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);

    if (imgIn.data == 0) {
        // Validar que la imagen exista
        printf("No pudo cargar la imagen %s\n", IMAGEN_ENTRADA);
        return 0;
    }

    // Definir intervalos para NUM_HILOS hilos
    int factor = imgIn.h / NUM_HILOS;
    int last = 1;
    for(int i = 0; i < NUM_HILOS; ++i) {
        intervalo[i][0] = last;
        if(i != (NUM_HILOS-1)) {
            intervalo[i][1] = last + factor-1;
        } else {
            intervalo[i][1] = imgIn.h - 1;
        }
        last = intervalo[i][1] + 1;
    }

    // Crear los Hilos
    int threadId[NUM_HILOS];
    pthread_t thread[NUM_HILOS];

    // Definir variables para medir el tiempo de ejecucion
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    
    // Crear los hilos
    for(int i = 0; i < NUM_HILOS; i++){
        threadId[i] = i;
        pthread_create(&thread[i], NULL, (void *)filter, &threadId[i]);
    }

    // Unir los hiloas
    int *retval;
    for(int i = 0; i < NUM_HILOS; i++){
        pthread_join(thread[i], (void **)&retval);
    }

    // Medir el tiempo
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    
    // Guardar la Imagen
    sod_img_save_as_png(imgOut, IMAGEN_SALIDA);

    // Liberar la Memoria
    sod_free_image(imgIn);
    sod_free_image(imgOut);

    // Mostrar el tiempo de ejecución
    printf("Time elapsed: %ld.%06ld using %d threads\n", (long int) tval_result.tv_sec, (long int) tval_result.tv_usec, NUM_HILOS);

    return 0;
}