# Practica 1 - S.O. I #
 Conjunto de prácicas de la asignatura de Sistemas Operativos I

Esta práctica consistirá en la implementación de un mini shell básico basado en el bash de Linux, y tendremos que superar 6 niveles para completar nuestra misión. 

El minishell tendrá las siguientes características:
   ## Admitirá  varios comandos internos que serán ejecutados por el propio shell y que no generarán la creación de un nuevo proceso:
        - cd directorio 
            Para cambiar de directorio actual de trabajo (chdir() al nuevo directorio especificado).
        
        - export variable=valor 
            Para asignar variables de entorno (setenv()). 
        
        - source nombre_fichero 
            Ejecuta las líneas de comandos del fichero nombre_fichero .

        - jobs 
            Muestra el PID, el estado y el nombre de los procesos que se están ejecutando en background o que han sido detenidos.

        - fg
            Envía un trabajo del background al foreground, o reactiva la ejecución en foreground de un trabajo que había sido detenido.

        - bg
            Reactiva un proceso detenido para que siga ejecutándose pero en segundo plano.

        - exit 
            Permite salir del mini shell (además de poder hacerlo con Ctrl+D).

   ## Admitirá comandos propios del shell estándard.
   El programa leerá una línea de comandos, la ejecutará con un nuevo proceso (fork() + execvp()) y permanecerá en espera a que el proceso termine          para leer la siguiente línea de comandos

   ## Admitirá redireccionamiento de la salida de un comando a un fichero, indicando “> fichero” al final de la línea

   ## Admitirá procesos en segundo plano (background). 
   Si al final de la línea de comandos hay un “&” el proceso se ejecutrá en segundo plano. Es decir, el shell no esperará a que acabe para seguir            leyendo los siguientes comandos (aunque avisará cuando el proceso haya acabado, de forma similar a cómo lo hace el shell estándard).

   ## Se admitirá la interrupción con Ctrl + C y Ctrl + Z.
