// передача числа между процессами через POSIX

// gcc client.c -o client -lrt

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mqueue.h>
#include <sys/stat.h>

#define QUEUE_NAME "/queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(int)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)

int main()
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mqd = mq_open(QUEUE_NAME, O_RDONLY, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("client: mq_open");
        exit(EXIT_FAILURE);
    }
    printf("client: POSIX descriptor = %u\n", mqd);

    int fd;
    ssize_t num_read = mq_receive(mqd, (char *)&fd, MSG_BUFFER_SIZE, NULL);
    if (num_read == -1)
    {
        perror("client: mq_receive");
        mq_close(mqd);
        exit(EXIT_FAILURE);
    }

    printf("client: Message received = length=%ld; body=%d\n", num_read, fd);

    if (mq_close(mqd) == -1)
    {
        perror("client: mq_close");
        exit(EXIT_FAILURE);
    }
    printf("client: Message queue closed\n");

    if (mq_unlink(QUEUE_NAME) == -1)
    {
        perror("client: mq_unlink");
        exit(EXIT_FAILURE);
    }
    printf("client: Message queue removed OK\n");

    return 0;
}