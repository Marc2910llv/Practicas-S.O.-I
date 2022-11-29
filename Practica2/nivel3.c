/*
-------------------------------------------------------------------------------------------------
Nivel 3 de la Práctica 2

En este nivel prepararemos nuestro minishell para que sea capaz de ejecutar, a través de la
llamada al sistema execvp(), los comandos que no sean internos. Cada vez que detectemos que se
trata de un comando externo, el minishell (proceso padre) creará un hijo que se encargue de él.
También seguiremos implementando comandos internos, en concreto el source.


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
#define N_JOBS 64

#define PROMPT '$'

#define DEBUGN1 0 // parse_args()
#define DEBUGN2 1 // check_internal()
#define DEBUGN3 0 // internal export & internal cd
#define DEBUGN4 1 // execute_line()
#define DEBUGN5 1 // internal source

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

struct info_job
{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’ (‘N’: ninguno, ‘E’: Ejecutándose y ‘D’: Detenido, ‘F’: Finalizado)
    char cmd[COMMAND_LINE_SIZE]; // línea de comando asociada
};

static struct info_job jobs_list[N_JOBS];
int resetear_joblist_0();
int actualizar_joblist_0(pid_t pid, char *line);

/// @brief resetear jobs_list[0]
int resetear_joblist_0()
{
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    strcpy(jobs_list[0].cmd, "\0");
    return 0;
}

/// @brief actualizar jobs_list[0] con el pid y la linia por parámetro
/// @param line
/// @param pid
int actualizar_joblist_0(pid_t pid, char *line)
{
    jobs_list[0].pid = pid;
    jobs_list[0].status = 'E';
    strcpy(jobs_list[0].cmd, line);
    return 0;
}

static char mi_shell[COMMAND_LINE_SIZE];
// variable global para guardar el nombre del minishell

/// @brief imprimir_prompt
/// Función auxiliar para imprimir PROMPT personalizado
void imprimir_prompt()
{
    printf(MAGENTA_T "%s" BLANCO_T ":", getenv("USER")); // imprimir usuario

    char prompt[COMMAND_LINE_SIZE];
    printf(CYAN_T "~%s", getcwd(prompt, COMMAND_LINE_SIZE)); // imprimir directorio

    printf(BLANCO_T "%c ", PROMPT);
    fflush(stdout);
}

/// @brief main
/// @param argc
/// @param argv
int main(int argc, char *argv[])
{
    char line[COMMAND_LINE_SIZE];

    resetear_joblist_0();

    strcpy(mi_shell, argv[0]);

    while (1)
    {
        if (read_line(line))
        { // !=NULL
            execute_line(line);
        }
    }
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

/// @brief ejecuta el comando entrado por consola
/// @param line
int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    pid_t pid;
    char *auxline = line;
    if (parse_args(args, line) > 0)
    {
        int rtn = check_internal(args);
        if (rtn == 0)
        {
            pid = fork();
            if (pid == 0) // HIJO
            {
                execvp(args[0], args);
                fprintf(stderr, ROJO_T "%s: no se encontró la orden\n" RESET, args[0]);
                exit(-1);
            }
            else if (pid > 0) // PADRE
            {
                actualizar_joblist_0(pid, auxline);
#if DEBUGN4
                fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d(%s)]\n" RESET, getpid(), mi_shell);
                fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d(%s)]\n" RESET, pid, auxline);
#endif
                wait(&pid);
#if DEBUGN4
                fprintf(stderr, GRIS_T "[execute_line()→ Proceso hijo %d(%s) finalizado con exit(), estado: %d]\n" RESET, pid, auxline, WEXITSTATUS(pid));
#endif
                resetear_joblist_0();
            }
            else // error
            {
                perror("Error fork\n");
            }
        }
    }
    return 0;
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
/// @return -1 si hay error, 1 si no lo hay para indicar que es comando interno
int internal_cd(char **args)
{
    if (args[1] != NULL)
    {
        if (chdir(args[1]) == -1)
        {
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
/// @return -1 si hay error, 1 si no lo hay para indicar que es comando interno
int internal_export(char **args)
{
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

/// @brief Se comprueban los argumentos y se muestra la sintaxis en caso de no ser correcta
/// @param args
/// @return -1 si hay error, 1 si no lo hay para indicar que es comando interno
int internal_source(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T "Error de sintaxis. Uso: source <nombre_fichero>\n" RESET);
        return -1;
    }

    FILE *fp = fopen(args[1], "r");
    if (fp == 0)
    {
        fprintf(stderr, ROJO_T "Error, no se encontró el fichero\n" RESET);
        return -1;
    }

    char str[COMMAND_LINE_SIZE];
    while (fgets(str, COMMAND_LINE_SIZE, fp) != NULL)
    {
        int i = 0;
        while (str[i] != '\n')
        {
            i++;
        }
        str[i] = '\0';
#if DEBUGN5
        fprintf(stderr, GRIS_T "\n[internal_source()→ LINE: %s]\n" RESET, str);
#endif
        fflush(stdout);
        execute_line(str);
    }

    if (fclose(fp) != 0)
    {
        perror("Error, no se ha cerrado el fichero correctamente\n");
        return -1;
    }
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