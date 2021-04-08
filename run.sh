if [ "$1" = "--debug" ]; then
    # Compilar CON flags de Debug
    gcc -Wshadow -Wall -Wimplicit-function-declaration -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG -o filtro filtro.c
else
    # Compilar SIN flags de Debug
    gcc -o filtro filtro.c
fi
# Ejecutar
./filtro miImagen.png miImagen_output.png 15 4