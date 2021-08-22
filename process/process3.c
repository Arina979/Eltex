#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void main()
{
    pid_t pid;

    char buffer[5];
    while (1)
    {
        printf("MENU: exit ls ps\n");
        memset(buffer, 0, 5);
        fgets(buffer, 5, stdin);

        if (!strcmp(buffer, "exit"))
            break;

        switch (pid = fork())
        {
        case -1:
            perror("fork");
            exit(1);
        case 0:
            if (!strcmp(buffer, "ls\n\0"))
            {
                execl("/bin/ls", "ls", (char *)0);
            }
            if (!strcmp(buffer, "ps\n\0"))
            {
                execl("/bin/ps", "ps", (char *)0);
            }
        default:
            pid = wait(NULL);
            if (pid == -1) 
            {
                perror("exec");
                exit(1);
            }
        }
    }
    return;
}
