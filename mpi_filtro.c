// mpicc -lm -o mpi mpi.c && mpirun -np 2 mpi img/input1.png img/output1.png 8

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include <mpi.h>

#include "sod/sod.h"

char* IMAGEN_ENTRADA;
char* IMAGEN_SALIDA;
int ARG;

float kernel[3][3] = {{-1,-1,-1}, 
                      {-1, 8,-1},
                      {-1,-1,-1}};

#define N 4096

float image[N][N];

#define MAX_INTERVAL 2048
int intervalo[MAX_INTERVAL][2]; // El i-th hilo vá desde el intervalo intervalo[i][0] hasta intervalo[i][1]

#define inf_float (100000000.0)

float** alloc_2d_int(int rows, int cols) {
    float** array = (float**) malloc(N * sizeof(float*));
    for(int i = 0; i < N; ++i) array[i] = (float*) malloc(N*sizeof(float*));

    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; ++j)
            array[i][j] = inf_float;

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; ++j) {
            array[i][j] = image[i][j];
        }
    }

    return array;
}

int main(int argc, char *argv[]) {

    if(argc < 4) {
        printf("Debe proporcionar 4 argumentos: [imagen de entrada] [imagen de salida] [argumento del filtro]");
        // Ejemplo: ./filtro.o img/input1.png img/output1.png 8 16
        exit(0);
    }


    int tasks, current_id, root = 0, tag = 1;
    MPI_Status status;

    MPI_Init(&argc , &argv);
    MPI_Comm_size( MPI_COMM_WORLD , &tasks );
    MPI_Comm_rank( MPI_COMM_WORLD , &current_id);

    // Ruta de la imagen de entrada: Ej: img/input1.png
    IMAGEN_ENTRADA = argv[1];

    // Ruta de la imagen de salida: Ej: img/output1.png
    IMAGEN_SALIDA = argv[2];

    // Argumento del filtro: Ej 8
    ARG = atof(argv[3]);
    kernel[1][1] = ARG;

    // printf("[Debug] tasks=%d, current_id=%d\n", tasks, current_id);


    /////////////////////////////////
    if(current_id == root) {
        for(int i = 0; i < N; ++i)
            for(int j = 0; j < N; ++j)
                image[i][j] = (float)(i);
    }
    /////////////////////////////////

    int h = 1024;
    int w = 1024;
    
    if(current_id == root) {

        // Definir intervalos
        int factor = h / tasks;
        int last = 1;
        for(int i = 0; i < tasks; ++i) {
            intervalo[i][0] = last;
            if(i != (tasks-1)) {
                intervalo[i][1] = last + factor-1;
            } else {
                intervalo[i][1] = h - 1;
            }
            last = intervalo[i][1] + 1;
        }
    }
    

    if(current_id == root) {
        for(int i = 1; i < tasks; ++i) {

            int from = intervalo[i][0];
            int to = intervalo[i][1];

            int limits[2] = {from, to};
            printf("[Root] Send: %d %d\n", intervalo[i][0], intervalo[i][1]);
            MPI_Send(&(limits[0]), 2, MPI_INT, i, tag, MPI_COMM_WORLD);

            float** board = alloc_2d_int(h, w);
            printf("[Root] Send Board\n");
            MPI_Send(&(board[0][0]), N*N, MPI_FLOAT, i, tag, MPI_COMM_WORLD);
        }


        for(int i = 1; i < tasks; ++i) {
            int from = intervalo[i][0];
            int to = intervalo[i][1];

            float** board = (float**) malloc(N * sizeof(float*));
            for(int i = 0; i < N; ++i) board[i] = (float*) malloc(N*sizeof(float*));

            MPI_Recv(&(board[0][0]), N*N, MPI_FLOAT, i, tag, MPI_COMM_WORLD, &status);
            printf("[Root] Recv Board\n");

            for(int y = from; y <= to; ++y) {
                for(int x = 0; x < N; ++x) {
                    if((board[x][y]+1.0) < inf_float) {
                        image[x][y] = board[x][y];
                    }
                }
            }
            printf("[Root] Terminó el procesamiento\n");
        }
    } else {
        int from, to;
        int limits[2];
        MPI_Recv(&(limits[0]), 2, MPI_INT, root, tag, MPI_COMM_WORLD, &status);
        from = limits[0];
        to = limits[1];

        printf("[Node] Recv limits{%d, %d}\n", from, to);

        float** board = (float**) malloc(N * sizeof(float*));
        for(int i = 0; i < N; ++i) board[i] = (float*) malloc(N*sizeof(float*));

        float** board_output = (float**) malloc(N * sizeof(float*));
        for(int i = 0; i < N; ++i) board_output[i] = (float*) malloc(N*sizeof(float*));

        MPI_Recv(&(board[0][0]), N*N, MPI_FLOAT, root, tag, MPI_COMM_WORLD, &status);
        printf("[Node] Recv Board\n");

        // PROCESSING
        
        for(int y = from; y <= to; ++y) {
            for(int x = 1; x < N-1; ++x) {
                if((board[x][y]+1.0) < inf_float) {

                    float sum = 0.0;
                    int is_good = 1;
                    
                    for(int ky = -1; ky <= 1; ++ky) {
                        for(int kx = -1; kx <= 1; ++kx) {
                            // Obtener pixel (Red) en la coordenada (x+kx, y+ky)
                            float val = board[x+kx][y+ky]; // R

                            if((val+1.0) > inf_float) {
                                is_good = 0;
                            }
                            
                            sum += kernel[ky+1][kx+1] * val;
                            //  kernel       pixeles de la imagen, px=Pixel

                            // [k1 k2 k3]    [px1 px2 px3]
                            // [k4 k5 k6]    [px4 px5 px6]
                            // [k7 k8 k9]    [px7 px8 px9]

                            // sum = k1*px1 + k2*px2 + ... + k9*px9
                        }
                    }

                    if(is_good > 0)
                        board_output[x][y] = abs(sum);
                }
            }
        }

        printf("[Node] Send Board node\n");
        MPI_Send(&(board_output[0][0]), N*N, MPI_FLOAT, root, tag, MPI_COMM_WORLD);

        // Free Memory
        for(int i = 0; i < N; ++i) {
            float* tmp1 = board[i];
            free(tmp1);

            float* tmp2 = board_output[i];
            free(tmp2);
        }

        printf("[Node] Terminó el procesamiento\n");
    }

    if(current_id == root) {
        printf("[Root] Guardar Imagen\n");
        // Guardar Imagen
        sod_img imgOut = sod_img_load_from_file(IMAGEN_SALIDA, SOD_IMG_COLOR);
        for(int y = 0; y <= h; ++y) {
            for(int x = 0; x < w; ++x) {
                float val = image[x][y];
                sod_img_set_pixel(imgOut, x, y, 0, abs(val)); // R
                sod_img_set_pixel(imgOut, x, y, 1, abs(val)); // G
                sod_img_set_pixel(imgOut, x, y, 2, abs(val)); // B
            }
        }
        sod_img_save_as_png(imgOut, IMAGEN_SALIDA);
        sod_free_image(imgOut);
    }
    

    MPI_Finalize();
    return 0;
}