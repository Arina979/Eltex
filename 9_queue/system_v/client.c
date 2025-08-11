// передача сообщения между процессами через SystemV

// gcc client.c -o client

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define SIZE_BUF 50

struct mymsg
{
    long mtype;  // приоритет очереди > 0
    char mtext[SIZE_BUF]; 
};

int main()
{
    key_t key;
    key = ftok("server.c", 1);
    if (key == -1)
    {
        perror("client: ftok");
        exit(EXIT_FAILURE);
    }
    printf("client: System V IPC key = %u\n", key);

    int msqid = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
    {
        perror("client: msgget");
        exit(EXIT_FAILURE);
    }
    printf("client: Message queue id = %u\n", msqid );

    // recv
    struct mymsg recv_msg; 
    int msg_len = msgrcv(msqid, &recv_msg, SIZE_BUF, 1, 0);
    if (msg_len == -1)
    {
        perror("client: msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("client: Message received = type=%ld; length=%d", recv_msg.mtype, msg_len);
        if (msg_len > 0)
    {
        printf("; body=%s", recv_msg.mtext);
    }
    printf("\n");

    // send
    struct mymsg send_msg; 
    send_msg.mtype = 2;
    strcpy(send_msg.mtext, "Hi server");
    if (msgsnd(msqid, &send_msg, SIZE_BUF, 0) == -1)
    {
        perror("client: msgsnd");
        exit(EXIT_FAILURE);
    }
    printf("client: Message sent successfully\n");

    return 0;
}