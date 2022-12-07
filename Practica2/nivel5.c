/*
-------------------------------------------------------------------------------------------------
Nivel 5 de la Práctica 2

En este nivel el mini shell (proceso padre) gestionará los procesos hijos en background
(que añadiremos a nuestro array de jobs_list).
También introduciremos el uso de una nueva señal, la SIGTSTP (producida al pulsar Ctrl + Z),
que al igual que en el nivel anterior, haremos que el hijo la ignore y que el padre la gestione
a través de un manejador, ctrlz. Con ese manejador, el mini shell enviará una señal de acción
similar, la SIGSTOP, al proceso en foreground (si éste existe y no se trata del mini shell
ejecutándose dentro del mini shell).

Además añadiremos un comando interno, jobs, que nos permitirá obtener una lista de los trabajos
existentes y su estado (necesitaremos funciones auxiliares para gestionar el array jobs_list:
añadir trabajos, eliminarlos, buscarlos...).


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
#define DEBUGN5 0 // internal source
#define DEBUGN6 1 // reaper
#define DEBUGN7 1 // ctrlc
#define DEBUGN8 1 // ctrlz

void imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);
int parse_args(char **args, char *line);
int is_background(char **args);
int jobs_list_add(pid_t pid, char status, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
int check_internal(char **args);

int n_pids = 1; // cantidad de trabajos detenidos o en background

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
        signal(SIGCHLD, reaper);
        signal(SIGINT, ctrlc);
        signal(SIGTSTP, ctrlz);
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
    char auxline[COMMAND_LINE_SIZE];
    memset(auxline, '\0', sizeof(auxline));
    strcpy(auxline, line);
    if (parse_args(args, line) > 0)
    {
        int rtn = check_internal(args);
        if (rtn == 0)
        {
            int bg = is_background(args);
            pid = fork();
            if (pid == 0) // HIJO
            {
                signal(SIGCHLD, SIG_DFL); // Asociamos SIGCHLD
                signal(SIGINT, SIG_IGN);  // Ignoramos SIGINT
                signal(SIGTSTP, SIG_IGN); // Ignoramos SIGSTP
                execvp(args[0], args);
                fprintf(stderr, ROJO_T "%s: no se encontró la orden\n" RESET, args[0]);
                exit(-1);
            }
            else if (pid > 0) // PADRE
            {
                if (bg == 0)
                {
                    actualizar_joblist_0(pid, auxline);
#if DEBUGN4
                    fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d(%s)]\n" RESET, getpid(), mi_shell);
                    fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d(%s)]\n" RESET, pid, auxline);
#endif
                    while (jobs_list[0].pid > 0)
                    {
                        pause();
                    }
                }
                else
                {
                    if (jobs_list_add(pid, 'E', auxline) == -1)
                    {
                        if (kill(pid, SIGTERM) == -1)
                        {
                            perror("Error: ");
                        }
                        fprintf(stderr, ROJO_T "No se ha añadido el trabajo, el número de trabajos es máximo\n" RESET);
                        return 0;
                    }
#if DEBUGN4
                    fprintf(stderr, GRIS_T "[execute_line()→ PID padre: %d(%s)]\n" RESET, getpid(), mi_shell);
                    fprintf(stderr, GRIS_T "[execute_line()→ PID hijo: %d(%s)]\n" RESET, pid, auxline);
#endif
                    fprintf(stderr, "[%d] %d    %c    %s\n", n_pids - 1, jobs_list[n_pids - 1].pid, jobs_list[n_pids - 1].status, jobs_list[n_pids - 1].cmd);
                }
            }
            else // error
            {
                perror("Error: ");
            }
        }
    }
    return 0;
}

/// @brief Manejador propio para la señal SIGCHLD
/// @param signum
void reaper(int signum)
{
    signal(SIGCHLD, reaper); // Asociamos SIGCHLD
    pid_t ended, status;
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (ended == jobs_list[0].pid)
        {
#if DEBUGN6
            char mensaje[3000];
            sprintf(mensaje, GRIS_T "\n[reaper()→ recibida señal %d(SIGCHLD)]\n" RESET, SIGCHLD);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            if (WIFEXITED(status))
            {
                sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en foreground (%s) ha finalizado con exit code %d]\n" RESET, ended, jobs_list[0].cmd, WEXITSTATUS(status));
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            else if (WIFSIGNALED(status))
            {
                if (WTERMSIG(status))
                {
                    sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en foreground (%s) ha terminado por señal %d]\n" RESET, ended, jobs_list[0].cmd, WTERMSIG(status));
                    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
                }
            }
#endif
            resetear_joblist_0();
        }
        else
        {
            int pos = jobs_list_find(ended);
#if DEBUGN6
            char mensaje[3000];
            sprintf(mensaje, GRIS_T "\n[reaper()→ recibida señal %d(SIGCHLD)]\n" RESET, SIGCHLD);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            if (WIFEXITED(status))
            {
                sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en background (%s) ha finalizado con exit code %d]\n" RESET, ended, jobs_list[pos].cmd, WEXITSTATUS(status));
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
                sprintf(mensaje, "Terminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[pos].cmd, pos, WEXITSTATUS(status));
                write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
            }
            else if (WIFSIGNALED(status))
            {
                if (WTERMSIG(status))
                {
                    sprintf(mensaje, GRIS_T "[reaper()→ Proceso hijo %d en background (%s) ha terminado por señal %d]\n" RESET, ended, jobs_list[pos].cmd, WTERMSIG(status));
                    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
                    sprintf(mensaje, "Terminado PID %d (%s) en jobs_list[%d] con status %d\n", ended, jobs_list[pos].cmd, pos, WTERMSIG(status));
                    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
                }
            }
#endif
            jobs_list_remove(pos);
        }
    }
}

/// @brief Manejador propio para la señal SIGINT
/// @param signum
void ctrlc(int signum)
{
    signal(SIGINT, ctrlc);
    pid_t pid = jobs_list[0].pid;
#if DEBUGN7
    char mensaje[3000];
    sprintf(mensaje, GRIS_T "\n[ctrlc()→ Soy el proceso con PID %d(%s), el proceso en foreground es %d(%s)]\n" RESET, getpid(), mi_shell, pid, jobs_list[0].cmd);
    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    sprintf(mensaje, GRIS_T "[ctrlc()→ recibida señal %d(SIGINT)]\n" RESET, SIGINT);
    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
    if (pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            if (kill(pid, SIGTERM) == -1)
            {
                perror("Error: ");
            }
#if DEBUGN8
            sprintf(mensaje, GRIS_T "[ctrlc()→ Señal %d(SIGTERM) enviada a %d(%s) por %d(%s)]\n" RESET, SIGTERM, pid, jobs_list[0].cmd, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
        }
        else
        {
#if DEBUGN7
            sprintf(mensaje, GRIS_T "[ctrlc()→ Señal %d(SIGTERM) no enviada por %d(%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGTERM, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
        }
    }
    else
    {
#if DEBUGN7
        sprintf(mensaje, GRIS_T "[ctrlc()→ Señal %d(SIGTERM) no enviada por %d(%s) debido a que no hay proceso en foreground]\n" RESET, SIGTERM, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
    }
}

/// @brief Manejador propio para la señal SIGTSTP
/// @param signum
void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    pid_t pid = jobs_list[0].pid;
#if DEBUGN8
    char mensaje[3000];
    sprintf(mensaje, GRIS_T "\n[ctrlz()→ Soy el proceso con PID %d, el proceso en foreground es %d(%s)]\n" RESET, getpid(), pid, jobs_list[0].cmd);
    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
    sprintf(mensaje, GRIS_T "[ctrlz()→ recibida señal %d(SIGTSTP)]\n" RESET, SIGTSTP);
    write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
    if (pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            if (kill(pid, SIGSTOP) == -1)
            {
                perror("Error: ");
            }
#if DEBUGN8
            sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d(SIGSTOP) enviada a %d(%s) por %d(%s)]\n" RESET, SIGSTOP, pid, jobs_list[0].cmd, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            printf("[%d]  %d    %c    %s\n", n_pids-1, jobs_list[n_pids - 1].pid, jobs_list[n_pids - 1].status, jobs_list[n_pids - 1].cmd);
            resetear_joblist_0();
        }
        else
        {
#if DEBUGN8
            sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d(SIGSTOP) no enviada por %d(%s) debido a que el proceso en foreground es el shell]\n" RESET, SIGSTOP, getpid(), mi_shell);
            write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
        }
    }
    else
    {
#if DEBUGN8
        sprintf(mensaje, GRIS_T "[ctrlz()→ Señal %d(SIGSTOP) no enviada por %d(%s) debido a que no hay proceso en foreground]\n" RESET, SIGSTOP, getpid(), mi_shell);
        write(2, mensaje, strlen(mensaje)); // 2 es el flujo stderr
#endif
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

/// @brief comprueba si el comando es en background
/// @param args
/// @return 1 si es background, 0 si es foreground
int is_background(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strchr(args[i], '&') != NULL)
        {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

/// @brief si es posible se añade un trabajo a la lista de trabajos
/// @param pid
/// @param status
/// @param cmd
int jobs_list_add(pid_t pid, char status, char *cmd)
{
    if (n_pids < N_JOBS) // si n_pids es menor a N_JOBS, aún podemos añadir más trabajos
    {
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].cmd, cmd);
        n_pids++;
        return 0;
    }
    else // no se pueden añadir más trabajos
    {
        return -1;
    }
}

/// @brief Busca en el array de trabajos el PID que recibe como argumento
/// @param pid
/// @return la posición del trabajo en el array o -1 si no se encuentra
int jobs_list_find(pid_t pid)
{
    for (int i = 0; i < N_JOBS; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            return i;
        }
    }
    return -1;
}

/// @brief elimina el trabajo de la lista que tiene como posición la pasada por parámetro
/// @param pos
int jobs_list_remove(int pos)
{
    n_pids--;
    jobs_list[pos].pid = jobs_list[n_pids].pid;
    jobs_list[pos].status = jobs_list[n_pids].status;
    strcpy(jobs_list[pos].cmd, jobs_list[n_pids].cmd);
    jobs_list[n_pids].pid = 0;
    jobs_list[n_pids].status = 'N';
    strcpy(jobs_list[n_pids].cmd, "\0");
    return 0;
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

/// @brief imprime la lista de trabajos
/// @param args
/// @return 1 para indicar que es comando interno
int internal_jobs(char **args)
{
    for (int i = 1; i < N_JOBS && jobs_list[i].pid > 0; i++)
    {
        fprintf(stderr, "[%d] %d    %c    %s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
    }
    return 1;
}

/// @brief
/// @param args
/// @return
int internal_bg(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_fg()→ Esta función enviará al foreground un proceso detenido o que esté en el background]\n" RESET);
#endif
    return 1;
}

/// @brief
/// @param args
/// @return
int internal_fg(char **args)
{
#if DEBUGN2
    fprintf(stderr, GRIS_T "[internal_bg()→ Esta función reactivará un proceso detenido y lo enviará al background]\n" RESET);
#endif
    return 1;
}