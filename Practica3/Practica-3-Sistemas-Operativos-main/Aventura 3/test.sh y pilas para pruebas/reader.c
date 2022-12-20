/*
Alberto Cugat Martín
Jaume Julià Vallespir
Rubén Palmer Pérez
*/

#include "my_lib.c"
#include "limits.h"

int main(int argc, char *argv[]){
    if(argc == 2){
        if (argv[1] == NULL){
            return -1;
        }
    }else{
        return -1;
    }

    struct my_stack *pila = my_stack_read(argv[1]);
    int len, sum, min, max = 0;
    float avg = 0;

    if(pila != NULL){
        max = INT_MIN;
        min = INT_MAX;
       
        len = my_stack_len(pila);
        if(len > 10){
            len = 10;
        }
        printf("Stack length: %d\n",len);
        for(int i = 0; i < len; i++){
            int aux = *((int *)my_stack_pop(pila)); 
            printf("%d \n",aux);

            if(aux < min){ 
                min = aux;
            }
            if(aux > max){ 
                max = aux;
            }
            sum = sum + aux;
        }
        avg = sum/len;
    }
    printf("Items: %d Sum: %d Min: %d Max: %d Average: %.3f \n", len, sum, min, max, avg);
}