process=0

if [ "$1" = "--init" ]; then
    rm -f -r sod/
    git clone https://github.com/symisc/sod.git
    exit 0
fi

# Compile library for Image Processing
# gcc sod/sod.c -lm -Ofast -march=native -std=c99 -o sod.o

# Compile Library
make

# Compile MPI
mpicc sod.o -D_DEFAULT_SOURCE -lm -o mpi_filtro mpi_filtro.c 

# mpicc -o mpi_filtro mpi_filtro.c && mpirun -np 2 mpi_filtro
if [ $process = 0 ]; then
    process=$(($1-0));
else
    process=1
fi


echo " "
echo "MPI - Running using $process process..."
echo " "

# Ejecutar

echo "MPI ($process process)" >> tmp.log
# [imagen de entrada] [imagen de salida] argumento del filtro

# mpirun -np $process ./mpi_filtro img/input1.png img/output1.png 8

time mpirun -np $process ./mpi_filtro img/input1.png img/output1.png 8
time mpirun -np $process ./mpi_filtro img/input2.png img/output2.png 8
time mpirun -np $process ./mpi_filtro img/input3.png img/output3.png 8

cat tmp.log
cat tmp.log >> cache.log
rm -f tmp.log
echo "#######################################" >> cache.log