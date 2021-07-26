#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct catalog
{
    char name[255];
    char surname[255];
    long int number;
} my_catalog[10];

void main()
{
    int menu;
    do
    {
        char buffer[255];
        printf("0: EXIT\n");
        printf("1: ADD\n");
        printf("2: DELETE\n");
        printf("3: SHOW\n");
        printf("4: SEARCH\n");
        printf("MENU: ");
        fgets(buffer, 255, stdin);
        while (!isdigit(buffer[0]))
        {
            printf("MENU: ");
            fgets(buffer, 255, stdin);
        }  
        menu = atoi(&buffer[0]);
        switch (menu)
        {
        case 0: //выход
            printf("EXIT\n");
            return;
        case 1: //добавить по номеру
        {
            printf("ADD\n");
            printf("num: ");
            int i;
            memset(buffer, 0, 255);
            fgets(buffer, 255, stdin);
            i = atoi(buffer);
            if (i < 0 || i > 10)
            {
                printf("error\n");
                break;
            }
            else
            {
                if (my_catalog[i].name[0] != '\0')
                {
                    printf("user already exists\n");
                    break;
                }
                printf("name: ");
                fgets(my_catalog[i].name, 255, stdin);
                printf("surname: ");
                fgets(my_catalog[i].surname, 255, stdin);
                char buff[6] = {};
                while ((buff == NULL) || (buff[0] == '\0'))
                {
                    printf("number: ");
                    fgets(buff, 6, stdin);
                    if (buff[5] != '\n' && buff[4] != '\0') fgets(buffer, 255, stdin);
                    int j = 0;
                    while(buff[j] != '\n' && buff[j] != '\0')
                    {
                        if (!isdigit(buff[j]))
                        {
                            memset(buff, 0, 6);
                            break;
                        }
                        j++;
                    }
                }
                buff[5] == '\0';
                printf("number: %s\n", buff);
                my_catalog[i].number = atoi(buff);
            }
        }
        break;
        case 2: //удалить по номеру
        {
            printf("DELETE\n");
            printf("num: ");
            char buff[11] = {};
            int i;
            fgets(buff, 255, stdin);
            if (buff[10] != '\n' && buff[9] != '\0') fgets(buffer, 255, stdin);
            i = atoi(buff);
            if (i < 0 || i > 10)
            {
                printf("error\n");
                break;
            }
            if (my_catalog[i].name[0] == '\0')
            {
                printf("user does not already exit\n");
                break;
            }
            memset(my_catalog[i].name, 0, 255);
            memset(my_catalog[i].surname, 0, 255);
            my_catalog[i].number = 0;
            printf("%d delete\n", i);
        }
        break;
        case 3: //посмотреть список
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
                    printf("number: %ld\n", my_catalog[i].number);
                }
            }
            if (k == 0)
                printf("catalog is empty\n");
        }
            break;
        case 4: //найти абонента по номеру телефона
        {
            printf("SEARCH\n");
            char buff[11] = {};
            while ((buff == NULL) || (buff[0] == '\0'))
            {
                printf("number: ");
                fgets(buff, 11, stdin);
                if (buff[10] != '\n' && buff[9] != '\0') fgets(buffer, 255, stdin);
                int i = 0;
                while (buff[i] != '\n' && buff[i] != '\0')
                {
                    if (!isdigit(buff[i]))
                    {
                        printf("%d\n", buff[i]);
                        memset(buff, 0, 11);
                        break;
                    }
                    i++;
                }
            }
            int k = 0;
            for (int i = 0; i < 10; ++i)
            {
                if (my_catalog[i].name[0] != '\0')
                    if (my_catalog[i].number == atoi(buff))
                    {
                        k++;
                        printf("num: %d\n", i);
                        printf("name: %s", my_catalog[i].name);
                        printf("surname: %s", my_catalog[i].surname);
                    }
            }
            if (k == 0) printf("not found\n");
        }
            break;
        default:
            printf("\n");
            break;
        }
        printf("\n");
    } while (menu);
}