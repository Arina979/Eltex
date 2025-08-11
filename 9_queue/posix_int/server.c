// передача числа между процессами через POSIX

// gcc server.c -o server -lrt

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define QUEUE_NAME "/queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(int)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)

int main()
{
    mq_unlink(QUEUE_NAME);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mqd = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, QUEUE_PERMISSIONS, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("server: mq_open");
        exit(EXIT_FAILURE);
    }
    printf("server: POSIX descriptor = %u\n", mqd);

    int fd = 12345;

    if (mq_send(mqd, (const char *)&fd, sizeof(fd), 0) == -1)
    {
        perror("server: mq_send");
        mq_close(mqd);
        exit(EXIT_FAILURE);
    }
    printf("server: Message sent successfully\n");

    if (mq_close(mqd) == -1)
    {
        perror("server: mq_close");
        exit(EXIT_FAILURE);
    }
    printf("server: Message queue closed\n");

    return 0;
}