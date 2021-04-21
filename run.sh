threads=0

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
    gcc sod.o -D_DEFAULT_SOURCE -fopenmp -lm -Ofast -march=native -std=c11 -Wshadow -Wall -o filtro.o filtro.c
    threads=$(($2-0));
else
    # Compilar SIN flags de Debug
    gcc sod.o -D_DEFAULT_SOURCE -fopenmp -lm -Ofast -march=native -std=c11 -o filtro.o filtro.c

    if [ $threads = 0 ]; then
        threads=$(($1-0));
    else
        threads=1
    fi
fi

echo " "
echo "Running using $threads threads ..."
echo " "

# Ejecutar

# [imagen de entrada] [imagen de salida] [argumento del filtro] [numero de hilos]
./filtro.o img/input1.png img/output1.png 8 $threads >> tmp.log
./filtro.o img/input2.png img/output2.png 8 $threads >> tmp.log
./filtro.o img/input3.png img/output3.png 8 $threads >> tmp.log
cat tmp.log
cat tmp.log >> cache.log
rm -f tmp.log
echo "#######################################" >> cache.log