
if [ "$1" = "--init" ]; then
    rm -f -r sod/
    git clone https://github.com/symisc/sod.git
    exit 0
fi

# Compile library for Image Processing
# gcc sod/sod.c -lm -Ofast -march=native -std=c99 -o sod.o

# Compile Library
make
# Compilar SIN flags de Debug

nvcc sod.o -o cuda_filtro cuda_filtro.cu 
echo " "
echo "CUDA"
echo " "

echo " " >>  tmp.log
echo "CUDA" >>  tmp.log
echo " " >>  tmp.log

# Ejecutar
# [imagen de entrada] [imagen de salida] [argumento del cuda_filtro] [numero de bloques] [numero de hilos por bloque]

for ((BLOCKS_GPU = 8 ; BLOCKS_GPU <= 64 ; BLOCKS_GPU *= 2));
do
    for ((THREADS_PER_BLOCK = 8; THREADS_PER_BLOCK <= 64 ; THREADS_PER_BLOCK *= 2));
    do
        # ./cuda_filtro [archivo de entrada] [archivo de salida] [argumento del cuda_filtro] [numero de bloques] [numero de hilos por bloque]
        ./cuda_filtro img/input1.png img/output1.png 8 "$BLOCKS_GPU" "$THREADS_PER_BLOCK"
    done
done

# echo "#######################################" >> tmp.log

for ((BLOCKS_GPU = 8; BLOCKS_GPU <= 64 ; BLOCKS_GPU *= 2));
do
    for ((THREADS_PER_BLOCK = 8; THREADS_PER_BLOCK <= 64 ; THREADS_PER_BLOCK *= 2));
    do
        # ./cuda_filtro [archivo de entrada] [archivo de salida] [argumento del cuda_filtro] [numero de bloques] [numero de hilos por bloque]
        ./cuda_filtro img/input2.png img/output2.png 8 "$BLOCKS_GPU" "$THREADS_PER_BLOCK"
    done
done

# echo "#######################################" >> tmp.log

for ((BLOCKS_GPU = 8; BLOCKS_GPU <= 64 ; BLOCKS_GPU *= 2)); do
    for ((THREADS_PER_BLOCK = 8; THREADS_PER_BLOCK <= 64 ; THREADS_PER_BLOCK *= 2)); do
        # ./cuda_filtro [archivo de entrada] [archivo de salida] [argumento del cuda_filtro] [numero de bloques] [numero de hilos por bloque]
        ./cuda_filtro img/input3.png img/output3.png 8 "$BLOCKS_GPU" "$THREADS_PER_BLOCK"
    done
done
echo "#######################################" >> tmp.log

cat tmp.log
cat tmp.log >> cache.log
rm -f tmp.log
rm -f cuda_filtro