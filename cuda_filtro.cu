// %%writefile cuda_filtro.cu
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "sod/sod.h"

#define MAX_H 4320
#define MAX_W 8192

#include <cuda_runtime.h>

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;
int NUM_HILOS;
int BLOQUES_GPU;
int HILOS_GPU;

#define MAX_INTERVAL 2048
int intervalo[MAX_INTERVAL][2]; // El i-th hilo vá desde el intervalo intervalo[i][0] hasta intervalo[i][1]

// ref: https://kth.instructure.com/courses/12406/pages/timing-your-kernel-cpu-timer-and-nvprof?module_item_id=169241
double cpuSecond() {
    struct timeval tp;
    gettimeofday(&tp,NULL);
    return ((double)tp.tv_sec + (double)tp.tv_usec*1.e-6);
}

#define INIT_KERNEL float kernel[3][3] = {{-1,-1,-1}, {-1, 8,-1}, {-1,-1,-1}};

__global__ 
void kernel(int d_intervalo[MAX_INTERVAL][2], float (*d_board)[MAX_W], float (*d_output)[MAX_W], int *d_W, int *d_blocks) {

    int ID = blockIdx.x * blockDim.x + threadIdx.x;

    if (ID < (*d_blocks)) {
        
        INIT_KERNEL;

        // Obtener intervalo, desde-hasta
        int from = d_intervalo[ID][0];
        int to = d_intervalo[ID][1];

        for(int y = from; y <= to; ++y) {
            for(int x = 1; x < (*d_W)-1; ++x) {

                float sum = 0.0;
                // Iterar la matrix 3x3 kernel
                for(int ky = -1; ky <= 1; ++ky) {
                    for(int kx = -1; kx <= 1; ++kx) {
                        // Obtener pixel (Red) en la coordenada (x+kx, y+ky)
                        float val = d_board[x+kx][y+ky]; // R
                        sum += kernel[ky+1][kx+1] * val;

                        //  kernel       pixeles de la imagen, px=Pixel

                        // [k1 k2 k3]    [px1 px2 px3]
                        // [k4 k5 k6]    [px4 px5 px6]
                        // [k7 k8 k9]    [px7 px8 px9]

                        // sum = k1*px1 + k2*px2 + ... + k9*px9
                    }
                }
                d_output[x][y] = abs(sum);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 6) {
        printf("Debe proporcionar 5 argumentos: [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de bloques] [numero de hilos por bloque]");
        // Ejemplo: ./filtro.o img/input1.png img/output1.png 8 16
        exit(0);
    }
    // Ruta de la imagen de entrada: Ej: img/input1.png
    IMAGEN_ENTRADA = argv[1];

    // Ruta de la imagen de salida: Ej: img/output1.png
    IMAGEN_SALIDA = argv[2];

    // Argumento del filtro: Ej 8
    ARG = atof(argv[3]);
    // kernel[1][1] = ARG;

    sod_img imgIn;
    sod_img imgOut;

    // w=4096 h=2160

    BLOQUES_GPU = atoi(argv[4]);

    HILOS_GPU = atoi(argv[5]);

    NUM_HILOS = BLOQUES_GPU * HILOS_GPU;

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

    printf("w=%d h=%d\n", imgIn.w, imgIn.h);

    // Crear variables con memoria dinamica
    // host board
    float (*board)[MAX_W] = (float (*)[MAX_W]) malloc(MAX_H*MAX_W*sizeof(float));

    for(int y = 0; y <= imgIn.h; ++y) {
        for(int x = 0; x < imgIn.w; ++x) {
            float val = sod_img_get_pixel(imgIn, x, y, 0); // RED
            board[x][y] = val;
        }
    }

    // device board
    float (*d_board)[MAX_W];
    cudaMalloc(&d_board, MAX_H*MAX_W*sizeof(float));
    cudaMemcpy(d_board, board, MAX_H*MAX_W*sizeof(float), cudaMemcpyHostToDevice);

    // Device output
    float (*d_output)[MAX_W];
    cudaMalloc(&d_output, MAX_H*MAX_W*sizeof(float));

    // Device Intervalo
    int (*d_intervalo)[2];
    cudaMalloc(&d_intervalo, MAX_INTERVAL*2*sizeof(int));
    cudaMemcpy(d_intervalo, intervalo, MAX_INTERVAL*2*sizeof(int), cudaMemcpyHostToDevice);

    // Device W
    int *d_W;
    int *tmp_W;
    int tmp = imgIn.w;
    tmp_W = &tmp;
    cudaMalloc(&d_W, sizeof(int));
    cudaMemcpy(d_W, tmp_W, sizeof(int), cudaMemcpyHostToDevice);

    int *d_blocks;
    int *hilos;
    hilos = &NUM_HILOS;
    cudaMalloc(&d_blocks, sizeof(int));
    cudaMemcpy(d_blocks, hilos, sizeof(int), cudaMemcpyHostToDevice);

    // Medir Tiempo de Ejecución
    double start = cpuSecond();

    // Run Cuda
    kernel<<<BLOQUES_GPU, HILOS_GPU>>>(d_intervalo, d_board, d_output, d_W, d_blocks);

    cudaDeviceSynchronize();
    double stop = cpuSecond();

    cudaMemcpy(board, d_output, MAX_H*MAX_W*sizeof(float), cudaMemcpyDeviceToHost);

    for(int y = 0; y <= imgIn.h; ++y) {
        for(int x = 0; x < imgIn.w; ++x) {
            float val = board[x][y];
            sod_img_set_pixel(imgOut, x, y, 0, abs(val)); // R
            sod_img_set_pixel(imgOut, x, y, 1, abs(val)); // G
            sod_img_set_pixel(imgOut, x, y, 2, abs(val)); // B
        }
    }
    
    // Guardar la Imagen
    sod_img_save_as_png(imgOut, IMAGEN_SALIDA);

    // Liberar la Memoria
    sod_free_image(imgIn);
    sod_free_image(imgOut);

    double time_elapsed = stop - start;

    // Mostrar el tiempo de ejecución
    printf("\nTime elapsed: %.8f sec using blocks_gpu=%d, threads_per_blocks=%d\n", time_elapsed, BLOQUES_GPU, HILOS_GPU);
    fflush(stdout);

    return 0;
}