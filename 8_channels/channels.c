// channels.c - простой командный интерпретатор c перенаправлением потока вывода в поток ввода

// ls -l | grep ch
// ls -la | cut -b 1-10

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIZE_STR 255
#define CMDS 2
#define CMD_COUNT_ARGS 10

struct command
{
    pid_t pid;
    char str[SIZE_STR];
    char *args[CMD_COUNT_ARGS];
    int count_args;
};

int main()
{
    char buffer[SIZE_STR];
    memset(buffer, 0, SIZE_STR);
    struct command cmds[CMDS];
    for (int i = 0; i < CMDS; i++)
    {
        memset(cmds[i].str, 0, SIZE_STR);
        for (int j = 0; j < CMD_COUNT_ARGS; j++)
        {
            cmds[i].args[i] = NULL;
        }
        cmds[i].count_args = 0;
    }

    printf("FORMAT: command args1 | command2 args2\n");

    while (1)
    {
        fgets(buffer, SIZE_STR, stdin);

        //разбиваем строку на команды
        char *ptr = strtok(buffer, "|");
        int i = 0;
        while (ptr != NULL)
        {
            strcpy(cmds[i++].str, (const char *)ptr);
            ptr = strtok(NULL, "|");
        }

        //разбиваем 1 строку на аргументы
        ptr = strtok(cmds[0].str, " ");
        i = 0;
        while (ptr != NULL)
        {
            cmds[0].args[i++] = ptr;
            cmds[0].count_args++;
            ptr = strtok(NULL, " ");
        }

        //разбиваем 2 строку на аргументы
        ptr = strtok(cmds[1].str, " \n");
        i = 0;
        while (ptr != NULL)
        {
            cmds[1].args[i++] = ptr;
            cmds[1].count_args++;
            ptr = strtok(NULL, " \n");
        }

        //создаем файловый дескриптор
        int file_pipes[2];
        if (pipe(file_pipes) != 0)
        {
            fprintf(stderr, "Pipe failure\n");
            exit(EXIT_FAILURE);
        }
        //создаем 1 процесс в родительском
        cmds[0].pid = fork();
        if (cmds[0].pid == -1)
        {
            fprintf(stderr, "Fork failure\n");
            exit(EXIT_FAILURE);
        }
        if (cmds[0].pid == 0)
        {
            //заменить стандартный вывод на файловый дискриптор
            close(STDOUT_FILENO);
            dup2(file_pipes[1], STDOUT_FILENO);
            execvp(cmds[0].args[0], cmds[0].args);
            exit(EXIT_FAILURE);
        }
        else
        {
            //ждем завершения 1 процесса
            int stat1;
            waitpid(cmds[0].pid, &stat1, 0);
            if (stat1 == -1)
            {
                perror("pid1");
                exit(EXIT_FAILURE);
            }

            close(file_pipes[1]);//EOF

            //создаем 2 процесс в родительском
            cmds[1].pid = fork();
            if (cmds[1].pid == -1)
            {
                fprintf(stderr, "Fork failure");
                exit(EXIT_FAILURE);
            }
            if (cmds[1].pid == 0)
            {
                //заменить стандартный ввод на файловый дискриптор
                close(STDIN_FILENO);
                dup2(file_pipes[0], STDIN_FILENO);
                execvp(cmds[1].args[0], cmds[1].args);
                exit(EXIT_FAILURE);
            }
            else
            {
                //ждем завершения 2 процесса
                int stat2;
                waitpid(cmds[1].pid, &stat2, 0);
                if (stat2 == -1)
                {
                    perror("pid2");
                    exit(EXIT_FAILURE);
                }

                close(file_pipes[0]);//EOF
            }
        }
    }
    return 0;
}
