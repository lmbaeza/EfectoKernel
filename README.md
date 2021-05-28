# Filtro Kernel
Procesar el filtro Kernel utilizando Computación Paralela

----

**Universidad Nacional de Colombia - Sede Bogotá**

 _**Computación Paralela**_

 **Docente:**   César Pedraza Bonilla

 **Estudiantes:**
 * Luis Miguel Báez Aponte - lmbaeza@unal.edu.co
 * Bryan David Velandia Parada - bdvelandiap@unal.edu.co
 * Camilo Ernesto Vargas Romero - camevargasrom@unal.edu.co

# Descargar

* Descargar Rama `main`
```shell
$ git clone https://github.com/lmbaeza/EfectoKernel.git
```

* Darle permiso de Ejecución
```shell
$ cd EfectoKernel
$ sudo chmod 777 run_posix.sh 
$ sudo chmod 777 run_openmp.sh 
```

# Descargar Libreria

```shell
$ ./run_posix.sh --init
o
$ ./run_openmp.sh --init
```

# Ejecutar Posix

```shell
$ ./run_posix.sh 4
# ./run_posix.sh [number of threads]
```
o Para `debugging`
```shell
$ ./run_posix.sh --debug 4
# ./run_posix.sh --debug [number of threads]
```

# Ejecutar OpenMP

```shell
$ ./run_openmp.sh 4
# ./run_openmp.sh [number of threads]
```
o Para `debugging`
```shell
$ ./run_openmp.sh --debug 4
# ./run_openmp.sh --debug [number of threads]
```