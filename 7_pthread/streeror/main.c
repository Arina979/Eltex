// streeror - по номеру ошибки возвращает строку, используется потокобезопасная библиотека 

// gcc -fPIC strerror.c -c
// gcc -shared strerror.o -o libstrerror.so
// gcc main.c -o main -L. -lstrerror -Wl,-rpath,.
// ./main

#define THREAD 5

#include "strerror.h"

void *thread(void *args)
{
    int *errno = (int *)args;
    printf("errno %d: %s\n", *errno, strerror(*errno));
}

int main ()
{
    pthread_t th[THREAD];
    int errno[THREAD];
    for (int i = 0; i < THREAD; i++)
    {
        errno[i] = i + 1;
        pthread_create(&th[i], NULL, thread, &errno[i]);
    }

    for (int i = 0; i < THREAD; i++)
    {
        pthread_join(th[i], NULL);
    }
}