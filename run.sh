if [ "$1" = "--init" ]; then
    rm -f -r sod/
    git clone https://github.com/symisc/sod.git
    exit 0
fi

# Compile library for Image Processing
# gcc sod/sod.c -lm -Ofast -march=native -std=c99 -o sod.o

# Compile Library
make

if [ "$1" = "--debug" ]; then
    # Compilar CON flags de Debug
    gcc sod.o -fopenmp -lm -Ofast -march=native -std=c11 -Wshadow -Wall -Wimplicit-function-declaration -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG -o filtro.o filtro.c
else
    # Compilar SIN flags de Debug
    gcc sod.o -D_DEFAULT_SOURCE -fopenmp -lm -Ofast -march=native -std=c11 -o filtro.o filtro.c
fi

echo " "

# Ejecutar

# [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de hilos]
./filtro.o img/input1.png img/output1.png 8 1
./filtro.o img/input2.png img/output2.png 8 1
./filtro.o img/input3.png img/output3.png 8 1
