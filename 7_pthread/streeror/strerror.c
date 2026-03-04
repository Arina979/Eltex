#include "strerror.h"

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t key;

void destr(void *buf)
{
    free(buf);
}

static void make_key()
{
    //позволяет получить уникальное хранилище для каждого потока. 
    //ключу соответствует указатель на уникальную область хранилища для конкретного потока. 
    pthread_key_create(&key, destr);
}

char *strerror(int errno)
{
    pthread_once(&once, make_key);//функцию выполнит только один поток
    //получить уникальный указатель в базе для каждого потока
    //возвращает 0, если нет записи
    char *buf = (char *)pthread_getspecific(key);
    if(buf == NULL)
    {
        buf = (char *)malloc(SIZE_STR);
        if(buf == 0)
        {
            printf("Allocation Failed");
            exit(EXIT_FAILURE);
        }
        //сохранить в базу указатель на ваше хранилище, вызывается один раз
        pthread_setspecific(key, (const void *)buf);
    }
    
    switch (errno)
    {
        case 1:
            strcpy(buf, "Operation not permitted");
            break;
        case 2:
            strcpy(buf, "No such file or directory");
            break;
        case 3:
            strcpy(buf, "No such process");
            break;
        case 4:
            strcpy(buf, "Interrupted system call");
            break;
        case 5:
            strcpy(buf, "I/O error");
            break;
        default:
            printf("\n");
            break;
    }
    return buf;
}