#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    bool addition = false;
    bool division = false;
    bool multiplication = false;
    bool subtraction = false;

    void **ext_library = (void **)malloc(4);

    int (*powerfunc)(int, int); 

    for (int i = 1; i < argc; i++)
    {
        int j;
        if (strcmp(argv[i], "libaddition.so") == 0)
        {
            if (addition) continue;
            addition = true;
            j = 0;
        }
        else if (strcmp(argv[i], "libsubtraction.so") == 0)
        {
            if (subtraction) continue;
            subtraction = true;
            j = 1;
        }
        else if (strcmp(argv[i], "libmultiplication.so") == 0)
        {
            if (multiplication) continue;
            multiplication = true;
            j = 2;
        }    
        else if (strcmp(argv[i], "libdivision.so") == 0)
        {
            if (division) continue;
            division = true;
            j = 3;
        }
        else 
        {
            printf("library:%s not supported\n", argv[i]);
            return 1;
        }

        ext_library[j] = dlopen(argv[i], RTLD_LAZY);
        if (!ext_library[j])
        {
            fprintf(stderr, "dlopen() error: %s\n", dlerror());
            return 1;
        };
    }

    int menu;
    do
    {
        char buffer[255];
        printf("0: EXIT\n");
        if (addition)
            printf("1: +\n");
        if (subtraction)
            printf("2: -\n");
        if (multiplication)
            printf("3: *\n");
        if (division)
            printf("4: \\\n");
        do
        {
            printf("MENU: ");
            fgets(buffer, 255, stdin);
        } while (!isdigit(buffer[0]));
        menu = atoi(&buffer[0]);
        int a, b; //трехзначные числа
        if (menu != 0)
        {
            char buff[4] = {};
            while ((buff == NULL) || (buff[0] == '\0'))
            {
                printf("a: ");
                fgets(buff, 4, stdin);
                if (buff[3] != '\n' && buff[2] != '\0')
                    fgets(buffer, 255, stdin);
                int j = 0;
                while (buff[j] != '\n' && buff[j] != '\0')
                {
                    if (!isdigit(buff[j]))
                    {
                        memset(buff, 0, 4);
                        break;
                    }
                    j++;
                }
            }
            buff[3] == '\0';
            a = atoi(buff);
            memset(buff, 0, 4);
            while ((buff == NULL) || (buff[0] == '\0'))
            {
                printf("b: ");
                fgets(buff, 4, stdin);
                if (buff[3] != '\n' && buff[2] != '\0')
                    fgets(buffer, 255, stdin);
                int j = 0;
                while (buff[j] != '\n' && buff[j] != '\0')
                {
                    if (!isdigit(buff[j]))
                    {
                        memset(buff, 0, 4);
                        break;
                    }
                    j++;
                }
            }
            buff[3] == '\0';
            b = atoi(buff);
        }
        if (menu == 0)
        {
            printf("EXIT\n");
            break;
        }
        if (menu == 1 && addition)
        {
            powerfunc = dlsym(ext_library[0], "addition");
            printf("%d + %d = %d\n\n", a, b, powerfunc(a, b));
            continue;
        }
        if (menu == 2 && subtraction)
        {
            printf("EXIT\n");
            powerfunc = dlsym(ext_library[1], "subtraction");
            printf("%d - %d = %d\n\n", a, b, powerfunc(a, b));
            continue;
        }
        if (menu == 3 && multiplication)
        {
            powerfunc = dlsym(ext_library[2], "multiplication");
            printf("%d * %d = %d\n\n", a, b, powerfunc(a, b));
            continue;
        }
        if (menu == 4 && division)
        {
            powerfunc = dlsym(ext_library[3], "division");
            printf("%d / %d = %d\n\n", a, b, powerfunc(a, b));
            continue;
        }
        else
        {
            printf("error\n\n");
            continue;
        }
    } while (menu);

    for (int i = 1; i < argc; i++)
    {
        int j;
        if (strcmp(argv[i], "libaddition.so") == 0)
        {
            if (addition) continue;
            addition = true;
            j = 0;
        }
        else if (strcmp(argv[i], "libsubtraction.so") == 0)
        {
            if (subtraction) continue;
            subtraction = true;
            j = 1;
        }
        else if (strcmp(argv[i], "libmultiplication.so") == 0)
        {
            if (multiplication) continue;
            multiplication = true;
            j = 2;
        }    
        else if (strcmp(argv[i], "libdivision.so") == 0)
        {
            if (division) continue;
            division = true;
            j = 3;
        }

        dlclose(ext_library[j]);
    }
};