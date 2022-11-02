/*
-------------------------------------------------------------------------------------------------
Nivel 1 de la Práctica 2

En este primer nivel crearemos el esqueleto de nuestro mini shell.
Tendremos un bucle infinito consistente en 2 acciones:
    - leer una línea de comandos desde el terminal
    - y (si esa linea es != NULL) ejecutarla.

Para poder ejecutar la línea de comandos, primero tendremos que descomponer esa línea en tokens
(elementos significativos), y analizar si el primero de ellos (el comando en cuestión) se trata
de:
    - un comando interno (los que implementaremos nosotros) o
    - uno externo (cuya ejecución externa delegaremos en un proceso hijo).


Autor: Marc Llobera Villalonga
-------------------------------------------------------------------------------------------------
*/

#include <errno.h>     //errno
#include <stdio.h>     //printf(), fflush(), fgets(), stdout, stdin, stderr, fprintf()
#include <stdlib.h>    //setenv(), getenv()
#include <string.h>    //strcmp(), strtok(), strerror()
#include <unistd.h>    //NULL, getcwd(), chdir()
#include <sys/types.h> //pid_t
#include <sys/wait.h>  //wait()
#include <signal.h>    //signal(), SIG_DFL, SIG_IGN, SIGINT, SIGCHLD
#include <fcntl.h>     //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h>  //S_IRUSR, S_IWUSR

#define RESET "\033[0m"
#define NEGRO_T "\x1b[30m"
#define NEGRO_F "\x1b[40m"
#define GRIS_T "\x1b[94m"
#define ROJO_T "\x1b[31m"
#define VERDE_T "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T "\x1b[34m"
#define MAGENTA_T "\x1b[35m"
#define CYAN_T "\x1b[36m"
#define BLANCO_T "\x1b[97m"
#define NEGRITA "\x1b[1m"

#define COMMAND_LINE_SIZE 1024

#define PROMPT '$'

void imprimir_prompt();
char *read_line(char *line);

/// @brief imprimir_prompt
/// Función auxiliar para imprimir PROMPT personalizado
void imprimir_prompt()
{
    printf(MAGENTA_T "%s" BLANCO_T ":", getenv("USER")); // imprimir usuario

    printf(CYAN_T "~%s", getenv("PWD")); // imprimir directorio

    printf(BLANCO_T "%c ", PROMPT);
    fflush(stdout);
}

/// @brief main
/// @param argc
/// @param argv
int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE]; // #define COMMAND_LINE_SIZE 1024
                                  // while (1)
    {
    if (read_line(line))
    { // !=NULL
      // execute_line(line);
    };
    };
}

char *read_line(char *line)
{
    imprimir_prompt();
    return line;
}
