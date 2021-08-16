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
float image_out[N][N];

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

    #define is_root(id) (root == id)
    #define is_node(id) (root != id)

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

    int h = 1024;
    int w = 1024;
    
    if(is_root(current_id)) {

        sod_img imgIn = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);

        if (imgIn.data == 0) {
            // Validar que la imagen exista
            printf("No pudo cargar la imagen %s\n", IMAGEN_ENTRADA);
            return 0;
        }

        h = imgIn.h;
        w = imgIn.w;

        for(int i = 0; i < N; ++i)
            for(int j = 0; j < N; ++j)
                image[i][j] = inf_float;
        
        for(int y = 0; y < h; ++y)
            for(int x = 0; x < w; ++x) {
                float val = sod_img_get_pixel(imgIn, x, y, 0); // RED
                image[x][y] = val;
            }

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

        sod_free_image(imgIn);
    }

    printf("h=%d, w=%d", h, w);
    

    if(is_root(current_id)) {

        float** board = alloc_2d_int(w, h);

        for(int i = 1; i < tasks; ++i) {

            int from = intervalo[i][0];
            int to = intervalo[i][1];
            int maxi = w;

            int limits[3] = {from, to, maxi};
            printf("[Root] Send: %d %d\n", from, to);
            MPI_Send(&(limits[0]), 3, MPI_INT, i, tag, MPI_COMM_WORLD);

            printf("[Root] Send Board\n");
            MPI_Send(&(board[0][0]), N*N, MPI_FLOAT, i, tag, MPI_COMM_WORLD);
        }

        for(int i = 1; i < tasks; ++i) {
            int from = intervalo[i][0];
            int to = intervalo[i][1];

            MPI_Recv(&(board[0][0]), N*N, MPI_FLOAT, i, tag, MPI_COMM_WORLD, &status);
            printf("[Root] Recv Board\n");

            /*for(int i = 0; i < N; ++i) {
                for(int j = 0; j < N; ++j) {
                    board[i][j] = 0.0;
                }
            }*/
            for(int y = from; y <= to; ++y) {
                for(int x = 0; x < w; ++x) {
                    image_out[x][y] = board[x][y];
                }
            }
            printf("[Root] Terminó el procesamiento\n");
        }
    }
    
    
    int from, to, maxi;

    float** board = (float**) malloc(N * sizeof(float*));
    for(int i = 0; i < N; ++i) board[i] = (float*) malloc(N*sizeof(float*));

    float** board_output = (float**) malloc(N * sizeof(float*));
    for(int i = 0; i < N; ++i) board_output[i] = (float*) malloc(N*sizeof(float*));

    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j) {
            board[i][j] = 0.0;
            board_output[i][j] = 0.0;
        }
    
    if(is_node(current_id)) {
        int limits[3];
        MPI_Recv(&(limits[0]), 3, MPI_INT, root, tag, MPI_COMM_WORLD, &status);
        from = limits[0];
        to = limits[1];
        maxi = limits[2];
        printf("[Node] Recv limits{%d, %d}\n", from, to);
    } else if(is_root(current_id)) {
        from = intervalo[root][0];
        to = intervalo[root][1];
        maxi = w;
    }

    if(is_node(current_id)) {
        MPI_Recv(&(board[0][0]), N*N, MPI_FLOAT, root, tag, MPI_COMM_WORLD, &status);
        printf("[Node] Recv Board\n");
    } else if(is_root(current_id)) {
        board = alloc_2d_int(w, h);
    }

    // PROCESSING
    
    for(int y = from; y <= to; ++y) {
        for(int x = 1; x < maxi-1; ++x) {

            float sum = 0.0;
            int is_good = 1;
            
            for(int ky = -1; ky <= 1; ++ky) {
                for(int kx = -1; kx <= 1; ++kx) {
                    // Obtener pixel (Red) en la coordenada (x+kx, y+ky)
                    float val = board[x+kx][y+ky]; // R

                    sum += kernel[ky+1][kx+1] * val;
                    //  kernel       pixeles de la imagen, px=Pixel

                    // [k1 k2 k3]    [px1 px2 px3]
                    // [k4 k5 k6]    [px4 px5 px6]
                    // [k7 k8 k9]    [px7 px8 px9]

                    // sum = k1*px1 + k2*px2 + ... + k9*px9
                }
            }

            board_output[x][y] = abs(sum);
        }
    }

    if(is_node(current_id)) {
        printf("[Node] Send Board node\n");
        MPI_Send(&(board_output[0][0]), N*N, MPI_FLOAT, root, tag, MPI_COMM_WORLD);
        printf("[Node] Terminó el procesamiento\n");
    } else if(is_root(current_id)) {
        for(int y = from; y <= to; ++y) {
            for(int x = 0; x < maxi; ++x) {
                image_out[x][y] = board_output[x][y];
            }
        }
    }

    if(is_root(current_id)) {
        printf("[Root] Guardar Imagen\n");
        // Guardar Imagen
        sod_img imgOut = sod_img_load_from_file(IMAGEN_ENTRADA, SOD_IMG_COLOR);
        for(int y = 0; y <= h; ++y) {
            for(int x = 0; x < w; ++x) {
                float val = image_out[x][y];
                sod_img_set_pixel(imgOut, x, y, 0, abs(val)); // R
                sod_img_set_pixel(imgOut, x, y, 1, abs(val)); // G
                sod_img_set_pixel(imgOut, x, y, 2, abs(val)); // B
            }
        }
        sod_img_save_as_png(imgOut, IMAGEN_SALIDA);
        sod_free_image(imgOut);
        printf("%s %s\n", IMAGEN_ENTRADA, IMAGEN_SALIDA);
    }

    // Free Memory
    for(int i = 0; i < N; ++i) {
        float* tmp1 = board[i];
        free(tmp1);
        float* tmp2 = board_output[i];
        free(tmp2);
    }

    MPI_Finalize();
    return 0;
}