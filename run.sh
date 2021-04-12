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
    gcc sod.o -lm -Ofast -march=native -std=c11 -Wshadow -Wall -Wimplicit-function-declaration -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG -o filtro.o filtro.c
else
    # Compilar SIN flags de Debug
    gcc sod.o -lm -Ofast -march=native -std=c11 -o filtro.o filtro.c
fi

# Ejecutar
./filtro.o input.png output.png 15 4