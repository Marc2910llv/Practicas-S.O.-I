/* 
-------------------------------------------------------------------------------------------------
 Librería de funciones que contiene las funciones análogas a las de <string.h> tales como:
 my_strlen(), my_strcmp(), my_strcpy(), my_strncpy(),  my_strcat() y my_strchr(). Además de
 la implementación de un gestor de pilas dinámico y genérico.

 Autor: Marc Llobera Villalonga
-------------------------------------------------------------------------------------------------
*/

#include "my_lib.h"
#include <errno.h>

//////////////////////////////////////////////////////////////////////
//////////////////////          RETO 1          //////////////////////
//////////////////////////////////////////////////////////////////////

/*
* Función: my_strlen
* ---------------------
* Calcula el número de bytes de la cadena apuntada por str.
* (No se incluye el carácter nulo de terminación '\0')
*
* · str: puntero que apunta a la cadena de la que se ha de encontrar la longitud.
*
* Return: la longitud de la cadena apuntada por str. No devuelve error.
*/
size_t my_strlen(const char *str)
{
    size_t len = 0;
    while (*str++)
    { // o while (*str++)!= '\0'
        len++;
    }
    return len;
}

/*
* Función: my_strcmp
* ---------------------
* Compara las cadenas que son apuntadas por los punteros str1 y str2.
* Se comparan carácter a carácter y al encontrar uno diferente devuelve
* la resta de los códigos ASCII de esos carácteres.
*
* · str1: puntero que apunta a la primera cadena que se quiere comparar.
* · str2: puntero que apunta a la segunda cadena que se quiere comparar.
*
* Return: un entero que indica que cadena es más grande o si son iguales.
*/
int my_strcmp(const char *str1, const char *str2)
{
    int cont = 0;
    int len1 = my_strlen(str1);
    int len2 = my_strlen(str2);
    if (len1 >= len2)
    {
        //while str1[cont] != '\0' y str1[cont] == str2[cont]
        while (str1[cont] && (str1[cont] == str2[cont]))
        {
            cont++;
        }
        return str1[cont] - str2[cont];
    }
    else
    {
        //while str2[cont] != '\0' y str1[cont] == str2[cont]
        while (str2[cont] && (str1[cont] == str2[cont]))
        {
            cont++;
        }
        return str1[cont] - str2[cont];
    }
}

/*
* Función: my_strcpy
* ---------------------
* Copia la cadena apuntada por el puntero src en la memoria apuntada
* por el puntero dest
*
* · dest: puntero que apunta a la cadena donde copiar el contenido.
* · src: puntero que apunta a la cadena que se quiere copiar.
*
* Return: dest. No devuelve error.
*/
char *my_strcpy(char *dest, const char *src)
{
    int cont = 0;
    int lendest = my_strlen(dest);
    while (src[cont]) //while src[cont] != '\0'
    {
        dest[cont] = src[cont];
        cont++;
    }
    while(cont<lendest)
    {
        dest[cont] = 0;
        cont++;
    }
    return dest;
}

/*
* Función: my_strcnpy
* ---------------------
* Copia la cadena apuntada por el puntero src en la memoria apuntada
* por el puntero dest. Solo se copiarán la cantidad de carácteres n.
* Al final rellenamos la cadena destino con 0s.
*
* · dest: puntero que apunta a la cadena donde copiar el contenido.
* · src: puntero que apunta a la cadena que se quiere copiar.
* · n: cantidad de carácteres que se quieren copiar.
*
* Return: dest. No devuelve error.
*/
char *my_strncpy(char *dest, const char *src, size_t n)
{
    int lensrc = my_strlen(src);
    int cont = 0;
    if (lensrc >= n)
    {
        for (; cont < n; cont++)
        {
            dest[cont] = src[cont];
        }
    }
    else //strlen(src) < n
    {
        for (; cont < n; cont++)
        {
            if (cont <= lensrc)
            {
                dest[cont] = src[cont];
            }
            else
            {
                dest[cont] = '\0';
            }
        }
    }
    return dest;
}

/*
* Función: my_strcat
* ---------------------
* Añade la cadena apuntada por el puntero src a la cadena apuntada por
* el puntero dest. El primer carácter de src sobreescribe el carácter
* nulo de dest.
*
* · dest: puntero que apunta a la cadena donde se añadirá src.
* · src: puntero que apunta a la cadena que se va a concatenar.
*
* Return: dest. No devuelve error.
*/
char *my_strcat(char *dest, const char *src)
{
    int lendest = my_strlen(dest);
    int lensrc = my_strlen(src);
    for (int i = 0; i <= lensrc; i++)
    {
        dest[lendest + i] = src[i];
    }
    return dest;
}

/*
* Función: my_strchr
* ---------------------
* Escanea la cadena apuntada por str buscando la primera ocurrencia
* del carácter que corresponde a c
*
* · str: puntero que apunta a la cadena que se va a escanear.
* · c: carácter que se quiere encontrar en la cadena.
*
* Return: str o NULL(si el carácter no se ha encontrado)
*/
char *my_strchr(const char *str, int c)
{
    char nuloAr[] = "(null)"; //array NULL
    char *nulo = nuloAr;      //puntero de la array NULL a devolver si no encontramos el carácter en la cadena
    while (*str != c)
    {
        str++;
        if (*str == 0) //si llegamos al final de la cadena
        {
            return nulo;
        }
    }
    return (char *)str;
}

//////////////////////////////////////////////////////////////////////
//////////////////////          RETO 2          //////////////////////
//////////////////////////////////////////////////////////////////////

/*
* Función: my_stack *my_stack_init
* ---------------------
* Reserva espacio para una variable de tipo struct my_stack, que 
* contendrá el puntero top al nodo superior de la pila y el tamaño 
* de los datos.
* 
* · size: contiene los datos del nodo apuntado por top.
* 
* Return: puntero a la pila inicializada.
*/
struct my_stack *my_stack_init(int size)
{
    struct my_stack *stack = malloc(sizeof(struct my_stack));
    if (stack == NULL)
    {
        perror("Error: ");
        return NULL;
    }
    stack->top = NULL;
    stack->size = size;
    return stack;
}

/*
* Función: my_stack_push
* ---------------------
* Inserta un nuevo nodo en los elementos de la pila (hay que 
* reservar espacio de memoria para este nuevo nodo).
*
* · stack: puntero que apunta al nuevo nodo introducido en la cadena.
* · data: puntero que apunta a los datos del nuevo nodo insertado.
* 
* Return: 0 o -1 (si ha habido un error).
*/
int my_stack_push(struct my_stack *stack, void *data)
{
    struct my_stack_node *new = malloc(sizeof(struct my_stack_node));
    if (new == NULL)
    {
        perror("Error: ");
        return -1;
    }
    new->data = data;
    new->next = stack->top;
    stack->top = new;
    return 0;
}

/*
* Función: *my_stack_pop
* ---------------------
* Elimina el nodo superior de los elementos de la pila (y libera 
* la memoria que ocupaba ese nodo). Devuelve el puntero a los 
* datos del elemento eliminado.
* 
* · stack: puntero que apuntaba a los datos del elemento eliminado (si existe).
*
* Return: data o NULL (si no existe un nodo superior).
*/
void *my_stack_pop(struct my_stack *stack)
{
    if (stack->top != NULL)
    {
        void *data = stack->top->data;
        struct my_stack_node *delete = stack->top;
        stack->top = stack->top->next;
        free(delete);
        return data;
    }
    else
    {
        return NULL;
    }
}

/*
* Función: my_stack_len
* ---------------------
* Recorre la pila y retorna el número de nodos totales 
* que hay en los elementos de la pila.
*
* · stack: puntero que va apuntando a todos los nodos de la cadena.
*
* Return: cont (nodos totales en la cadena).
*/
int my_stack_len(struct my_stack *stack)
{
    int cont = 0;
    struct my_stack_node *actual = stack->top;
    if (stack->top != NULL)
    {
        cont++;
        while (actual->next != NULL)
        {
            cont++;
            actual = actual->next;
        }
    }
    return cont;
}

/*
* Función: my_stack_purge
* ---------------------
* Recorre la pila liberando la memoria (free(data)) que se había 
* reservado para cada uno de los datos y la de cada nodo.
* 
* · stack: puntero que va apuntando los nodos de la cadena para su 
*          posterior liberacion de la memoria.
*
* Return: remuvedByte (num de bytes liberados).
*/
int my_stack_purge(struct my_stack *stack)
{
    int remuvedByte = 0;
    void *data = my_stack_pop(stack);
    while (data != NULL)
    {
        remuvedByte += sizeof(struct my_stack_node) + stack->size;
        free(data);
        data = my_stack_pop(stack);
    }
    remuvedByte += sizeof(struct my_stack);
    free(stack);
    return remuvedByte;
}

/*
* Función: my_stack_write
* ---------------------
* Almacena los datos de la pila en el fichero indicado por filename.
*
* · stack: puntador que apunta al contenido de una pila auxilar. 
* · filename: indica el fichero que es tratado.
*
* Return: my_stack_len(stack) (num elementos almacenados) o -1 (Error).
*/
int my_stack_write(struct my_stack *stack, char *filename)
{

    /*Uso de los siguientes flags para abrir el fichero*/
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        perror("Error: ");
        return -1;
    }
    int size = stack->size;
    struct my_stack *auxiliar = my_stack_init(stack->size);
    if (write(fd, &size, sizeof(int)) == 0)
    {
        perror("Error: ");
        return -1;
    }
    struct my_stack_node *auxnode = stack->top;
    while (auxnode != NULL)
    {
        my_stack_push(auxiliar, auxnode->data);
        auxnode = auxnode->next;
    }
    struct my_stack_node *writtingnode = auxiliar->top;
    while (writtingnode != NULL)
    {
        if (write(fd, writtingnode->data, size) == 0)
        {
            perror("Error: ");
            return -1;
        }
        writtingnode = writtingnode->next;
    }
    close(fd);
    return my_stack_len(stack);
}

/*
* Función: my_stack *my_stack_read
* ---------------------
* Lee los datos de la pila almacenados en el fichero.
* 
* · filename: indica el fichero del que se han leido 
*             los datos de las pilas que tiene almacenadas. 
*
* Return: read () devuelve 0 o NULL (error).
*/
struct my_stack *my_stack_read(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("Error: ");
        return NULL;
    }
    int size = 0;
    if (read(fd, &size, sizeof(int)) == 0)
    {
        return NULL;
    }
    void *data = malloc(size);
    if (data == NULL)
    {
        perror("Error: ");
        return NULL;
    }
    struct my_stack *readingstack = my_stack_init(size);
    while (read(fd, data, size) > 0)
    {
        my_stack_push(readingstack, data);
        data = malloc(size);
        if (data == NULL)
        {
            perror("Error: ");
            return NULL;
        }
    }
    close(fd);
    return readingstack;
}