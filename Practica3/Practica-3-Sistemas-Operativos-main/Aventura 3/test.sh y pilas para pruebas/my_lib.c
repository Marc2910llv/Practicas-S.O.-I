/*
Alberto Cugat Martín
Jaume Julià Vallespir
Rubén Palmer Pérez
*/

#include <stdio.h>
#include <stdlib.h>
#include "my_lib.h"

/*********************************************************************************************************************************/
/*********************************************************************************************************************************/
/*                                                   SEMANA 4                                                                    */
/*********************************************************************************************************************************/
/*********************************************************************************************************************************/

/*
La función strlen() calcula el nº de bytes de la cadena apuntada por str, sin incluir el carácter nulo de terminación ‘\0’ .
Devuelve la longitud de la cadena apuntada por str. No devuelve error.
*/
size_t my_strlen(const char *str){
    int length = 0;
    for (int i = 0; str[i] != '\0' ; i++){
        length++;
    }
    return length;
}

/*
La función strcmp() compara las cadenas apuntadas por str1 y str2.
Devuelve un entero:
< 0 indica que str1 < str2.
> 0 indica que str2 < str1.
= 0 indica que str1 = str2.
No se basa en la longitud sino en los códigos ASCII. Cuando encuentra uno diferente reporta el resultado calculado como la resta de los códigos 
ASCII de los caracteres diferentes.
*/
int my_strcmp(const char *str1, const char *str2){
    int i;
    for(i = 0;((str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0'));i++){}
    return str1[i] - str2[i];
}

/*
La función strcpy() copia la cadena apuntada por src (con el carácter de terminación ‘\0’) en la memoria apuntada por dest.
Devuelve el puntero dest. No devuelve error.
*/
char *my_strcpy(char *dest, const char *src){ 
    int i;
    for (i = 0; src[i] != '\0' ; i++){
            dest[i] = src[i]; 
    }
    dest[i] = '\0';
    return dest;
}

/*
La función strncpy() copia n caracteres de la cadena apuntada por src (con el carácter de terminación ‘\0’) en la memoria apuntada por dest.
Devuelve el puntero dest. No devuelve error.
En caso de que strlen(src) < n, el resto de la cadena apuntada por dest ha de ser rellenado con 0s. 
*/
char *my_strncpy(char *dest, const char *src, size_t n){
    int srcLength = my_strlen(src);
    int i;
    for(i = 0; (i < n) && (i < srcLength); i++){
        dest[i] = src[i]; 
    }

    while (i < n){
        dest[i] = '\0';
        i++;
    }
    
    return dest;
}

/*
La función strcat() añade la cadena apuntada por src (terminada con el carácter nulo) a la cadena apuntada por dest 
(también terminada con el carácter nulo). El primer carácter de src sobreescribe el carácter nulo de dest. 
Devuelve el puntero dest. No devuelve error.
*/
char *my_strcat(char *dest, const char *src){
    int i;
    for (i = 0; dest[i] != '\0'; i++){} 
    for (int n = 0; src[n] != '\0'; n++){
        dest[i] = src[n];
        i++;
    }
    dest[i] = '\0';

    return dest;
}

/*********************************************************************************************************************************/
/*********************************************************************************************************************************/
/*                                                   SEMANAS 5-6                                                                 */
/*********************************************************************************************************************************/
/*********************************************************************************************************************************/
int write_file(struct my_stack *stack, struct my_stack_node *pointer, int *fichero);
void recursive_write(struct my_stack *stack, struct my_stack_node *pointer, int *contador, int *fichero);
void *pop_node(struct my_stack *stack);

/*
Reserva espacio para una variable de tipo struct my_stack, que contendrá el puntero al nodo superior de la pila y el tamaño de los datos, 
e inicializa esos valores con NULL, como valor del puntero al nodo superior de la pila, y con el tamaño de datos que nos pasan como parámetro.
Devuelve un puntero a la pila inicializada.
*/
struct my_stack *my_stack_init (int size){
    struct my_stack *puntero;
    puntero = malloc(sizeof(struct my_stack));
    puntero -> top = NULL;    
    puntero -> size = size;

    return puntero;
}

/*
Inserta un nuevo nodo en los elementos de la pila (hay que reservar espacio de memoria para él). El puntero a los datos de ese nodo nos lo pasan 
como parámetro. Devuelve 0 si ha ido bien, -1 si hubo error (o EXIT_SUCCESS o EXIT_FAILURE).
*/
int my_stack_push (struct my_stack *stack, void *data){
    struct my_stack_node *pointer;
    pointer = malloc(sizeof(struct my_stack_node));
    if(!pointer){
        return -1;
    }
    if ((stack != NULL) && (stack -> size > 0)) {
        pointer -> data = data;
        pointer -> next = stack -> top;
        stack -> top = pointer;        
    } else {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

/*
Elimina el nodo superior de los elementos de la pila (y libera la memoria que ocupaba ese nodo). Devuelve el puntero a los datos del elemento eliminado. 
Si no existe nodo superior (pila vacía), retorna NULL.
*/
void *my_stack_pop (struct my_stack *stack){   
    void *data = NULL;

    if ((stack -> top != NULL) && (stack != NULL)) {
       data = pop_node(stack);

    } else {
        return NULL;
    }
    
    return data;
}

/*
Recorre la pila y retorna el número de nodos totales que hay.
*/
int my_stack_len (struct my_stack *stack){
    int length = 0;

    if (stack != NULL) {
        if( stack -> top != NULL){
            struct my_stack_node *pointer;
            pointer = stack -> top;
            length = 1;
            
            while (pointer -> next != NULL){
                pointer = pointer -> next;
                length++;
            }
        }       
    } else {
        return -1;
    }
    return length;
}

/*
Recorre la pila liberando la memoria que habíamos reservado para cada uno de los datos y nodos. Finalmente libera también 
la memoria que ocupa la pila. Devuelve el número de bytes liberados. 
*/
int my_stack_purge (struct my_stack *stack){
    int freeBytes = 0;

    if(stack != NULL){
        while(stack -> top != NULL){
            freeBytes += stack -> size + sizeof(struct my_stack_node);
            pop_node(stack);
        }

        free(stack);
        freeBytes += sizeof(struct my_stack);
    } else {
        return -1;

    }

    return freeBytes;
}

/*
Almacena los datos de la pila en el fichero indicado por filename. Devuelve el número de elementos almacenados, -1 si hubo error. 
*/
int my_stack_write (struct my_stack *stack, char *filename){ 

    int contador = 0;
    if(stack != NULL){
        if(stack -> top != NULL){
            struct my_stack_node *pointer;
            pointer = malloc(sizeof(struct my_stack_node));
            if (!pointer){
                return -1;
            }
            pointer = stack -> top;

            int fichero = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if(fichero < 0){ 
                close(fichero);
                return -1;
            }

            void *size = &stack -> size;
            int bytes = write(fichero, size, sizeof(stack->size)); 
            if (bytes != sizeof(stack->size)){
                close(fichero);
                return -1;
            }

            recursive_write(stack, pointer, &contador, &fichero); 
            
            write_file(stack, pointer, &fichero);
            contador++;
            close(fichero);
        }
    } else {
        return -1;
    }

    return contador;
}

/*
Lee los datos de la pila almacenados en el fichero (el primer valor es el tamaño de los datos), inicializa la pila, la reconstruye en memoria y retorna el puntero a la pila.
Si hubo error, retorna NULL.
*/
struct my_stack *my_stack_read (char *filename){
    struct my_stack *stack;
    int size;
    void *data;
    int fichero = open(filename, O_RDONLY , S_IRUSR);
    
    if(fichero < 0){
        close(fichero);
        return NULL;
    }
    
    read(fichero, &size, sizeof(int));
    stack = my_stack_init(size);
    data = malloc(size);
    if (!data){
        return NULL;
    }
    while(read(fichero, data, size) != 0){
        my_stack_push(stack, data);
        data = malloc(size);
        if (!data){
            return NULL;
        }
    }
    
    close(fichero);
    
    return stack;
}

/*********************************************************************************************************************************/
/*********************************************************************************************************************************/
/*                                                 AUXILIARY PROGRAMS                                                            */
/*********************************************************************************************************************************/
/*********************************************************************************************************************************/

/*
Función recursiva que escribe todos los elementos de una pila a un fichero pasado por parámetero. Devuelve la cantidad de elementos en la pila 
*/
void recursive_write(struct my_stack *stack, struct my_stack_node *pointer, int *contador, int *fichero){

    if(pointer -> next != NULL) {
        pointer = pointer -> next;
        *contador = *contador + 1;
        recursive_write(stack, pointer, contador, fichero);
        write_file(stack, pointer, fichero);
    }

}

/*
Metodo para escribir datos pasados por referencia en un archivo determinado. Devuelve -1 en caso de error
*/
int write_file(struct my_stack *stack, struct my_stack_node *pointer, int *fichero){

    int bytes = 0;
    void *data = pointer -> data;
    bytes = write(*fichero, data, stack->size);
    
    if(bytes != stack->size){
        close(*fichero);
        return -1;
    }

    return 0;
}

/*
Codigo necesario para eliminar un nodo. Reutilizable
*/
void *pop_node (struct my_stack *stack){
    struct my_stack_node *pointer;
    void *data = NULL;

    pointer = stack -> top;
    data = pointer -> data;
    stack -> top = pointer -> next;
    free(pointer);
        
    return data;
}