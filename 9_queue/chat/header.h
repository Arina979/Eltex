// чат с общей комнатой

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SERVER_QUEUE_NAME "/my_server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MSG_SIZE 30
#define QUEUE_SIZE 16

#define MAX_CLIENTS_COUNT 10
#define MAX_MSGS_COUNT 30

enum signal
{
    INIT = 0,
    MSG,
    EXIT,
    EXIT_SERVER
};
struct msg
{
    char buffer[MSG_SIZE];
    char user_name[MSG_SIZE];
    char queue_name[QUEUE_SIZE];
    enum signal sign;
};
#define MAX_MSG_SIZE sizeof(struct msg)
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10