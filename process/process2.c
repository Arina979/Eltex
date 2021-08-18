#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void main()
{
    ////0
    //4///1
    //5//2//3

    pid_t pid;

    switch (pid = fork())
    {
    case -1:
        perror("fork1");
        exit(1);
    case 0:
        printf("CHILD 1: PID -- %d PPID -- %d\n", getpid(), getppid());

        switch (pid = fork())
        {
        case -1:
            perror("fork2");
            exit(1);
        case 0:
            printf("CHILD 2: PID -- %d PPID -- %d\n", getpid(), getppid());
            exit(0);
        default:
            //1 процеесс-потомок
            switch (pid = fork())
            {
            case -1:
                perror("fork3");
                exit(1);
            case 0:
                printf("CHILD 3: PID -- %d PPID -- %d\n", getpid(), getppid());
                exit(0);
            default:
                //завершение 2 и 3 потомка
                pid = wait(NULL);
                if (pid == -1)
                {
                    perror("exec");
                    exit(1);
                }
                pid = wait(NULL);
                if (pid == -1)
                {
                    perror("exec");
                    exit(1);
                }
                exit(0); //завершение 1 потомка
            }
        }
    default:
        //родитель
        printf("PARENT:  PID -- %d\n", getpid());
        switch (pid = fork())
        {
        case -1:
            perror("fork4");
            exit(1);
        case 0:
            printf("CHILD 4: PID -- %d PPID -- %d\n", getpid(), getppid());
            switch (pid = fork())
            {
            case -1:
                perror("fork5");
                exit(1);
            case 0:
                printf("CHILD 5: PID -- %d PPID -- %d\n", getpid(), getppid());
                exit(0);
            default:
                //завершение 5 потомка
                if (pid == -1)
                {
                    perror("exec");
                    exit(1);
                }
                exit(0); //завершение 4 потомка
            }
        default:
            //завершение 1 и 4 потомка
            pid = wait(NULL);
            if (pid == -1)
            {
                perror("exec");
                exit(1);
            }
            pid = wait(NULL);
            if (pid == -1)
            {
                perror("exec");
                exit(1);
            }
            exit(0); //завершение родителя
        }
    }
}