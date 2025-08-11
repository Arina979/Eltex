// калькулятор с динамической загрузкой библиотек из указанной папки

// rm -rf ./plugins
// mkdir plugins

// static lib
// gcc -fPIC -c addition.c -o ./plugins/addition.o
// gcc -fPIC -c subtraction.c -o ./plugins/subtraction.o
// gcc -fPIC -c multiplication.c -o ./plugins/multiplication.o
// gcc -fPIC -c division.c -o ./plugins/division.o

// dinamic lib
// gcc -shared ./plugins/addition.o -o ./plugins/libaddition.so
// gcc -shared ./plugins/subtraction.o -o ./plugins/libsubtraction.so
// gcc -shared ./plugins/multiplication.o -o ./plugins/libmultiplication.so
// gcc -shared ./plugins/division.o -o ./plugins/libdivision.so

// gcc main.c -o test -L. -ldl -Wl,-rpath,./plugins
// ./test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>

#define SIZE_INT 11
#define SIZE_STR 255
#define DIRECTOTY "plugins"

struct lib
{
    int (*func)(int, int);
    void *handle;
    char funcname[SIZE_STR];
};

int loadLibs(struct lib **libs)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(DIRECTOTY);
    int n = 0;
    if (d) 
    {
        while ((dir = readdir(d)) != NULL) 
        {
            int len = strlen(dir->d_name);
            if(dir->d_name[0] == '.' || strstr(dir->d_name, ".so") == NULL)
                continue;

            struct lib l = {0};
            void *sym = NULL;
            l.handle = dlopen(dir->d_name, RTLD_NOW);
            if (!l.handle)
            {
                fprintf(stderr, "Dlopen error: %s\n", dlerror());
                goto library_init_fail;
            };
            sym = dlsym(l.handle, "funcname");
            if (sym == NULL)
            {
                fprintf(stderr, "Dlsym error: %s\n", dlerror());
                goto library_init_fail;
            }
            strcpy(l.funcname, (char *)sym);
            l.func = dlsym(l.handle, l.funcname);
            if (l.func == NULL)
            {
                fprintf(stderr, "Dlsym error: %s\n", dlerror());
                goto library_init_fail;
            }
            
            if(*libs == NULL)
            {
                *libs = malloc(sizeof(struct lib));
                if(*libs == 0)
                {
                    fprintf(stderr, "Malloc error: %s\n", dlerror());
                    exit(EXIT_FAILURE);
                }
                n++;
            }
            else
            {
                n++;
                *libs = realloc(*libs, sizeof(struct lib) * n);
                if(*libs == NULL)
                {
                    fprintf(stderr, "Realloc error: %s\n", dlerror());
                    exit(EXIT_FAILURE);
                }
            }
            (*libs)[n-1].func = l.func;
            (*libs)[n-1].handle = l.handle;
            strcpy((*libs)[n-1].funcname, (char *)sym);
            printf("Library %s with function %s loaded\n", dir->d_name, (*libs)[n-1].funcname);
            continue;

            library_init_fail:
            if(l.handle)
            {
                dlclose(l.handle);
                l.handle = NULL;
                continue;
            }
        }
        closedir(d);
    }
    return n;
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

void menu(struct lib *libs, int n)
{
    if(n == 0)
    {
        printf("Libs not found\n");
        return;
    }
    int menu = n;
    do
    {
        for (int i = 0; i < n; i++)
        {
            printf("%d: %s\n", i, libs[i].funcname);
        }
        printf("%d: EXIT\n", n);
        printf("num from 0 to %d: ", n);

        menu = inputInt();
        if(menu < 0 || menu > n)
        {
            printf("Error num\n");
            continue;
        }

        int a = 0, b = 0;
        if(menu != n)
        {
            printf("a: ");
            a = inputInt();
            printf("b: ");
            b = inputInt();
            printf("%s = %d\n", libs[menu].funcname, libs[menu].func(a, b));
        }
    } while (menu != n);
    printf("EXIT\n");
}

int main()
{
    struct lib *libs = 0;
    int n = loadLibs(&libs);
    menu(libs, n);

    for (int i = 0; i < n; i++)
    {
        if(libs[i].handle)
        {
            dlclose(libs[i].handle);
            libs[i].handle = NULL;
        }
    }
    free(libs);
};