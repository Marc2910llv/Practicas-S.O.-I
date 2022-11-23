/*
-------------------------------------------------------------------------------------------------
Nivel 2 de la Práctica 2

En este nivel implementaremos los comandos internos cd y export de nuestro mini shell,
y también veremos cómo tratar los errores que pueden producir las llamadas a sistema,
y en general cómo utilizar el flujo estándar de errores para mostrar cualquier error de
nuestros programas.


Autor: Marc Llobera Villalonga
-------------------------------------------------------------------------------------------------
*/

#define _POSIX_C_SOURCE 200112L

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
#define ARGS_SIZE 64

#define PROMPT '$'

#define DEBUGN1 0 // parse_args()
#define DEBUGN2 0 // check_internal()
#define DEBUGN3 0 // internal export & internal cd

void imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);

// COMANDOS INTERNOS --------------
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_bg(char **args);
int internal_fg(char **args);
// --------------------------------

/// @brief imprimir_prompt
/// Función auxiliar para imprimir PROMPT personalizado
void imprimir_prompt()
{
    printf(MAGENTA_T "%s" BLANCO_T ":", getenv("USER")); // imprimir usuario

    char prompt[COMMAND_LINE_SIZE];                          // #define COMMAND_LINE_SIZE 1024
    printf(CYAN_T "~%s", getcwd(prompt, COMMAND_LINE_SIZE)); // imprimir directorio

    printf(BLANCO_T "%c ", PROMPT);
    fflush(stdout);
}

/// @brief main
/// @param argc
/// @param argv
int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE]; // #define COMMAND_LINE_SIZE 1024
    while (1)
    {
        if (read_line(line))
        { // !=NULL
            execute_line(line);
        };
    };
}

/// @brief leer linea
/// @param line
/// @return puntero a la líea leida
char *read_line(char *line)
{
    imprimir_prompt();
    char *pLine = fgets(line, COMMAND_LINE_SIZE, stdin);
    if (pLine)
    {
        // ELiminamos el salto de línea sustituyéndolo por el \0
        char *pos = strchr(line, 10);
        if (pos != NULL)
        {
            *pos = '\0';
        }
    }
    else
    { // ptr==NULL por error o fin de fichero
        printf("\r");
        if (feof(stdin))
        { // Ctrl+D
            fprintf(stderr, "Ctrl+D\n");
            exit(0);
        }
    }
    return pLine;
}

/// @brief
/// @param line
/// @return
int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    parse_args(args, line);
    if (check_internal(args) == -1)
    {
        perror("Error\n");
    }
}

/// @brief trocea la línea en tokens
/// @param args
/// @param line
/// @return número de tokens troceados
int parse_args(char **args, char *line)
{
    int i = 0;
    args[i] = strtok(line, " \t\n\r");

#if DEBUGN1
    fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
#endif

    while (args[i] && args[i][0] != '#')
    {
        i++;
        args[i] = strtok(NULL, " \t\n\r");

#if DEBUGN1
        fprintf(stderr, GRIS_T "[parse_args()→ token %i: %s]\n" RESET, i, args[i]);
#endif
    }

    if (args[i])
    {
        args[i] = NULL;

#if DEBUGN1
        fprintf(stderr, GRIS_T "[parse_args()→ token %i corregido: %s]\n" RESET, i, args[i]);
#endif
    }

    return i;
}

/// @brief comprueba si comando introducido es interno
/// @param args
/// @return devuelve 0 si no es un comando interno, o 1 al contrario a través de la llamada a las funciones correspondientes
int check_internal(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        return internal_cd(args);
    }
    if (strcmp(args[0], "export") == 0)
    {
        return internal_export(args);
    }
    if (strcmp(args[0], "source") == 0)
    {
        return internal_source(args);
    }
    if (strcmp(args[0], "jobs") == 0)
    {
        return internal_jobs(args);
    }
    if (strcmp(args[0], "bg") == 0)
    {
        return internal_bg(args);
    }
    if (strcmp(args[0], "fg") == 0)
    {
        return internal_fg(args);
    }
    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    return 0; // no es un comando interno
}

/// @brief Comando cd para cambiar de directorio
/// @param args
/// @return -1 si hay error, 1 si no lo hay
int internal_cd(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_cd()→ Esta función cambiará de directorio]\n" RESET);
#endif
    if (args[1] != NULL)
    {
        if (chdir(args[1]) == -1)
        {
            perror("Error en internal_cd");
            return -1;
        }
#if DEBUGN3
        char prompt[COMMAND_LINE_SIZE];
        fprintf(stderr, GRIS_T "[internal_cd()→ PWD: %s]\n" RESET, getcwd(prompt, COMMAND_LINE_SIZE));
#endif
    }
    else
    {
        if (chdir(getenv("HOME")) == -1)
        {
            perror("Error en internal_cd");
            return -1;
        }

#if DEBUGN3
        fprintf(stderr, GRIS_T "[internal_cd()→ PWD: %s]\n" RESET, getenv("HOME"));
#endif
    }
    return 1;
}

/// @brief comando export para dar valores nuevos a variables de entorno
/// @param args
/// @return -1 si hay error, 1 si no lo hay
int internal_export(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_export()→Esta función asignará valores a variablescd de entorno]\n" RESET);
#endif
    char *nombre;
    char *valor;
    nombre = strtok(args[1], "=");
    valor = strtok(NULL, "");
    if (nombre == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return -1;
    }
    else
    {
#if DEBUGN3
        fprintf(stderr, GRIS_T "[internal_export()→ nombre: %s]\n" RESET, nombre);
        fprintf(stderr, GRIS_T "[internal_export()→ valor: %s]\n" RESET, valor);
#endif
    }
    if (valor == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: export Nombre=Valor\n" RESET);
        return -1;
    }
    else
    {
        setenv(valor, nombre, 1); ///////////////*****************Preguntar porque no funciona*************//////////////////
#if DEBUGN3
        fprintf(stderr, GRIS_T "[internal_export()→ antiguo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
        fprintf(stderr, GRIS_T "[internal_export()→ nuevo valor para %s: %s]\n" RESET, nombre, getenv(nombre));
#endif
    }
    return 1;
}

int internal_source(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_source()→Esta función ejecutará un fichero de líneas de comandos]\n" RESET);
#endif
    return 1;
}

int internal_jobs(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_jobs()→Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
#endif
    return 1;
}

int internal_bg(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_fg()→ Esta función enviará al foreground un proceso detenido o que esté en el background]\n" RESET);
#endif
    return 1;
}

int internal_fg(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_bg()→ Esta función reactivará un proceso detenido y lo enviará al background]\n" RESET);
#endif
    return 1;
}