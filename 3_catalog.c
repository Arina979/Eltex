// 3_catalog.c - статический и динамический телефонные справочники

// gcc 3_cataloc.c -o catalog

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#define SIZE_INT 11
#define SIZE_STR 255

struct catalog
{
    char name[SIZE_STR];
    char surname[SIZE_STR];
    int number;
};

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
void inputStr(char *str)
{
    memset(str, 0, SIZE_STR);
    fgets(str, SIZE_STR, stdin);
    if (strchr(str,'\n') == NULL)
    {
        str[SIZE_STR - 1] = '\n';
        while ((getchar()) != '\n');
    }
}

void staticMenu()
{
    struct catalog my_catalog[10] = {0};
    char menu;
    do
    {
        printf("0: EXIT\n");
        printf("1: ADD\n");
        printf("2: DELETE\n");
        printf("3: SHOW\n");
        printf("4: SEARCH\n");
        printf("MENU: ");
        menu = inputChar();
        switch (menu)
        {
        case '0': //выход
            printf("EXIT\n");
            return;
        case '1': //добавить по номеру
        {
            printf("ADD\n");
            printf("num from 0 to 9: ");
            int i = inputInt();
            if (i < 0 || i > 10)
            {
                printf("Error\n");
                break;
            }
            else
            {
                if (my_catalog[i].name[0] != '\0')
                {
                    printf("User already exists\n");
                    break;
                }
                printf("name: ");
                inputStr(my_catalog[i].name);
                printf("surname: ");
                inputStr(my_catalog[i].surname);
                printf("number: ");
                my_catalog[i].number = inputInt();
            }
        }
        break;
        case '2': //удалить по номеру
        {
            printf("DELETE\n");
            printf("num from 0 to 9: ");
            int i = inputInt();
            if (i < 0 || i > 10)
            {
                printf("error\n");
                break;
            }
            if (my_catalog[i].name[0] == '\0')
            {
                printf("User does not already exit\n");
                break;
            }
            memset(my_catalog[i].name, 0, SIZE_STR);
            memset(my_catalog[i].surname, 0, SIZE_STR);
            my_catalog[i].number = 0;
            printf("%d delete\n", i);
        }
        break;
        case '3': //посмотреть список
        {
            printf("SHOW\n");
            int k = 0;
            for (int i = 0; i < 10; ++i)
            {
                if (my_catalog[i].name[0] != '\0')
                {
                    k++;
                    printf("num: %d\n", i);
                    printf("name: %s", my_catalog[i].name);
                    printf("surname: %s", my_catalog[i].surname);
                    printf("number: %d\n", my_catalog[i].number);
                }
            }
            if (k == 0)
                printf("catalog is empty\n");
        }
            break;
        case '4': //найти абонента по номеру телефона
        {
            printf("SEARCH\n");
            printf("number: ");
            int number = inputInt();
            int k = 0;
            for (int i = 0; i < 10; ++i)
            {
                if (my_catalog[i].name[0] != '\0')
                    if (my_catalog[i].number == number)
                    {
                        k++;
                        printf("num: %d\n", i);
                        printf("name: %s", my_catalog[i].name);
                        printf("surname: %s", my_catalog[i].surname);
                    }
            }
            if (k == 0) printf("Not found\n");
        }
            break;
        default:
            printf("\n");
            break;
        }
        printf("\n");
    } while (menu);
}

void dinamicMenu()
{
    struct catalog *my_catalog = NULL;
    char menu;
    int n = 0;
    do
    {
        printf("0: EXIT\n");
        printf("1: ADD\n");
        printf("2: DELETE\n");
        printf("3: SHOW\n");
        printf("4: SEARCH\n");
        printf("MENU: ");
        menu = inputChar();
        switch (menu)
        {
        case '0': //выход
            printf("EXIT\n");
            free(my_catalog);
            return;
        case '1': //добавить
        {
            printf("ADD\n");
            if(n == 0)
            {
                my_catalog = malloc(sizeof(struct catalog));
                if(my_catalog == 0)
                {
                    perror("Malloc error");
                    exit(EXIT_FAILURE);
                }
                n++;
            }
            else
            {
                n++;
                my_catalog = realloc(my_catalog, sizeof(struct catalog) * n);
                if(my_catalog == 0)
                {
                    perror("Realloc error");
                    exit(EXIT_FAILURE);
                }
            }
            printf("name: ");
            inputStr(my_catalog[n-1].name);
            printf("surname: ");
            inputStr(my_catalog[n-1].surname);
            printf("number: ");
            my_catalog[n-1].number = inputInt();
            printf("add element %d\n", n);
        }
        break;
        case '2': //удалить по номеру
        {
            printf("DELETE\n");
            printf("num: ");
            int i = inputInt();
            if(i > n-1)
            {
                printf("User does not already exit\n");
                break;
            }

            for (int j = i; j < n-1; j++)
            {
                memcpy(my_catalog[j].name, my_catalog[j+1].name, SIZE_STR);
                memcpy(my_catalog[j].surname, my_catalog[j+1].surname, SIZE_STR);
                my_catalog[j].number = my_catalog[j+1].number;
            }
            n--;
            my_catalog = realloc(my_catalog, sizeof(struct catalog) * n);
            if(my_catalog == 0)
            {
                perror("Realloc error");
                exit(EXIT_FAILURE);
            }
            printf("%d delete\n", i);
        }
        break;
        case '3': //посмотреть список
        {
            printf("SHOW\n");
            for (int i = 0; i < n; i++)
            {
                printf("num: %d\n", i);
                printf("name: %s", my_catalog[i].name);
                printf("surname: %s", my_catalog[i].surname);
                printf("number: %d\n", my_catalog[i].number);
            }
        }
        break;
        case '4': //найти абонента по номеру телефона
        {
            printf("SEARCH\n");
            printf("number: ");
            int number = inputInt();
            int k = 0;
            for (int i = 0; i < n; i++)
            {
                if (my_catalog[i].name[0] != '\0')
                    if (my_catalog[i].number == number)
                    {
                        k++;
                        printf("num: %d\n", i);
                        printf("name: %s", my_catalog[i].name);
                        printf("surname: %s", my_catalog[i].surname);
                    }
            }
            if (k == 0) printf("Not found\n");
        }
            break;
        default:
            printf("\n");
            break;
        }
        printf("\n");
    } while (menu);
}

void main()
{
    char menu;
    do
    {
        printf("0: STATIC\n");
        printf("1: DINAMIC\n");
        printf("MENU: ");
        menu = inputChar();
        switch (menu)
        {
        case '0':
            static_menu();
            return;
        case '1':
            dinamic_menu();
            return;
        default:
            printf("\n");
            break;
        }
        printf("\n");
    } while (menu);
}
