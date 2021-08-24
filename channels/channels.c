#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1;
    pid_t pid2;

    char buffer[255];
    char str[2][255];
    char arg1[10][10];
    char arg2[10][10];

    printf("FORMAT: command args1 | command2 args2\n");

    while (1)
    {
        memset(buffer, 0, 255);
        memset(str[0], 0, 255);
        memset(str[1], 0, 255);
        for (int j = 0; j < 10; j++)
        {
            memset(arg1[j], 0, 255);
            memset(arg2[j], 0, 255);
        }
        fgets(buffer, 255, stdin);

        //разбиваем строку на команды
        char *ptr = strtok(buffer, "|");
        int i = 0;
        while (ptr != NULL)
        {
            strcpy(str[i++], (const char *)ptr);
            ptr = strtok(NULL, "|");
        }

        //разбиваем 1 строку на аргументы
        ptr = strtok(str[0], " ");
        i = 0;
        while (ptr != NULL)
        {
            strcpy(arg1[i++], (const char *)ptr);
            ptr = strtok(NULL, " ");
        }

        //разбиваем 2 строку на аргументы
        ptr = strtok(str[1], " \n");
        i = 0;
        while (ptr != NULL)
        {
            strcpy(arg2[i++], (const char *)ptr);
            ptr = strtok(NULL, " \n");
        }

        int file_pipes[2];
        if (pipe(file_pipes) != 0)
        {
            fprintf(stderr, "Pipe failure\n");
            exit(EXIT_FAILURE);
        }
        //создаем 1 процесс в родительском
        pid1 = fork();
        if (pid1 == (pid_t)-1)
        {
            fprintf(stderr, "Fork failure\n");
            exit(EXIT_FAILURE);
        }
        if (pid1 == (pid_t)0)
        {
            //заменить стандартный вывод на файловый дискриптор
            close(1);
            dup2(file_pipes[1], 1);
            if (arg1[1][0] != '\0')
                execlp(arg1[0], arg1[0], arg1[1], NULL);
            else
                execlp(arg1[0], arg1[0], NULL);
            exit(EXIT_FAILURE);
        }
        else
        {
            int stat1;
            waitpid(pid1, &stat1, 0);
            if (stat1 == -1)
            {
                perror("pid1");
                exit(1);
            }

            close(file_pipes[1]);//EOF

            //создаем 2 процесс в родительском
            pid2 = fork();
            if (pid2 == (pid_t)-1)
            {
                fprintf(stderr, "Fork failure");
                exit(EXIT_FAILURE);
            }
            if (pid2 == (pid_t)0)
            {
                //заменить стандартный ввод на файловый дискриптор
                close(0);
                dup2(file_pipes[0], 0);
                if (arg2[1][0] != '\0')
                    execlp(arg2[0], arg2[0], arg2[1], NULL);
                else
                    execlp(arg2[0], arg2[0], NULL);
                exit(EXIT_FAILURE);
            }
            else
            {

                int stat2;
                waitpid(pid2, &stat2, 0);
                if (stat2 == -1)
                {
                    perror("pid2");
                    exit(1);
                }
            }
        }
    }
    return 0;
}
