
// простой калькулятор с одной библиотекой

// static lib
// gcc my_calc.c -c
// ar rc libcalc.a my_calc.o
// gcc main.c -o test -L. -lcalc
// ./test

// dinamic lib
// gcc -fPIC my_calc.c -c
// gcc -shared my_calc.o -o libcalc.so
// gcc main.c -o test -L. -lcalc -Wl,-rpath,.
// ./test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_calc.h"

#define SIZE_INT 11

typedef enum { false, true } bool;

char inputChar()
{
    char c = getchar();
    while ((getchar()) != '\n') ;
    return c;
}
int inputInt()
{
    char buffer[SIZE_INT] = {0};
    char *copy = NULL;
    int num = 0;
    bool succsess = false;
    while(!succsess)
    {
        fgets(buffer, SIZE_INT, stdin);
        if (strchr(buffer,'\n') == NULL)
        {
            while ((getchar()) != '\n');

            if(buffer[0] != '+' && buffer[0] != '-')
            {
                buffer[0] == ' ';//переполнение
            }
        }

        copy = strtok(buffer,"\n");

        char *endptr;
        num = strtol(copy, &endptr, 10);
        if (endptr == copy)
        {
            printf("No digits were found\n");
        } 
        else if (*endptr != '\0')
        {
            printf("Invalid character: %c\n", *endptr);
        }
        else
        {
            succsess = true;
        }
    }
    return num;
}

int main()
{
    char menu;
    do
    {
        printf("0: EXIT\n");
        printf("1: +\n");
        printf("2: -\n");
        printf("3: *\n");
        printf("4: /\n");
        printf("MENU: ");
        menu = inputChar();
        int a = 0, b = 0;
        if(menu != '0')
        {
            printf("a: ");
            a = inputInt();
            printf("b: ");
            b = inputInt();
        }
        switch (menu)
        {
        case '0':
            printf("EXIT\n");
            return 0;
        case '1':
            printf("a + b = %d\n", addition(a, b));
            break;
        case '2':
            printf("a - b = %d\n", subtraction(a, b));
            break;
        case '3':
            printf("a * b = %d\n", multiplication(a, b));
            break;
        case '4':
            printf("a / b = %d\n", division(a, b));
            break;
        default:
            printf("\n");
            break;
        }
        printf("\n");
    } while (menu);
};