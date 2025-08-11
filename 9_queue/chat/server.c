// чат с общей комнатой

#include "header.h"

struct client
{
    bool enable;
    char user_name[MSG_SIZE];
    char queue_name[QUEUE_SIZE];
    mqd_t qd;
}clients[MAX_CLIENTS_COUNT];

mqd_t qd_server;
bool interrupt = false;

void close_server()
{
    interrupt = true;

    struct msg mymsg = {0};
    strcpy(mymsg.user_name, "server");
    strcpy(mymsg.queue_name, SERVER_QUEUE_NAME);
    mymsg.sign = EXIT_SERVER;
    for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
    {
        if (clients[i].enable)
        {
            if (mq_send(clients[i].qd, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
            {
                perror("Server: Not able to send message to client");
                continue;
            }
        }
    } 

    if (mq_close(qd_server) == -1)
    {
        perror("Server: mq_close");
        exit(1);
    }

    if (mq_unlink(SERVER_QUEUE_NAME) == -1)
    {
        perror("Server: mq_unlink");
        exit(1);
    }

    printf("Server: bye\n");
}

int main(int argc, char **argv)
{
    if (signal(SIGINT, close_server) == SIG_ERR)
    {
        perror("Server: ERROR opening signal");
        exit(1);
    }

    mqd_t qd_server; // дескриптор очереди

    for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
        clients[i].enable = false;
    int count_cli = 0;

    struct msg ring_buff_msgs[MAX_MSGS_COUNT];
    int start_buf = 0;
    int cur_buf = -1;
    int count_buf = 0;

    printf("Server: start\n");

    // создаем очередь для получения сообщений от клиентов
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    mq_unlink(SERVER_QUEUE_NAME);
    if ((qd_server = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1)
    {
        perror("Server: mq_open");
        exit(1);
    }
    printf("Server: mq_open\n");

    struct msg mymsg;
    struct timespec ts;
    while (!interrupt)
    {
        memset(&mymsg, 0, sizeof(struct msg));
        // получить самое старое сообщение с наивысшим приоритетом
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1; // Set a 1-second timeout
        if(mq_timedreceive(qd_server, (char *)&mymsg, MSG_BUFFER_SIZE, NULL, &ts) == -1)
        //if (mq_receive(qd_server, (char *)&mymsg, MSG_BUFFER_SIZE, NULL) == -1)
        {
            if (errno != ETIMEDOUT)
            {
                perror("Server: mq_receive");
                exit(1);
            }
            else
            {
                continue;
            }
        }
        printf("Server: message received (signal: %d, queue: %s, user: %s, buffer: %s\n)", mymsg.sign, mymsg.queue_name, mymsg.user_name, mymsg.buffer);

        switch (mymsg.sign)
        {
        case INIT:
        {
            printf("Server: INIT: user: %s\n", mymsg.user_name);

            mqd_t qd;
            if ((qd = mq_open(mymsg.queue_name, O_WRONLY)) == -1)
            {
                perror("Server: mq_open (client)");
                break;
            }

            int current = -1;
            if (count_cli + 1 <= MAX_CLIENTS_COUNT)
            {
                // есть свободное место для подключения клиента
                for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
                {
                    if (!clients[i].enable)
                    {
                        current = i;
                        break;
                    }
                }
                if(current < 0)
                {
                    perror("Server: incorrect current");
                    break;
                }
                clients[current].qd = qd;
                strcpy(clients[current].user_name, mymsg.user_name);
                strcpy(clients[current].queue_name, mymsg.queue_name);
                clients[current].enable = true;
                count_cli++;
            }
            else
            {
                // отказ
                perror("Server: too many clients");

                strcpy(mymsg.user_name, "server");
                strcpy(mymsg.queue_name, SERVER_QUEUE_NAME);
                mymsg.sign = EXIT_SERVER;
                if (mq_send(qd, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
                {
                    perror("Server: Not able to send message to client");
                    break;
                }
                break;
            }

            // для присоеденившегося клиента
            // отправка ранее присоеденившихся клиентов
            struct msg msg;
            msg.sign = INIT;
            for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
            {
                if (clients[i].enable && i != current)
                {
                    strcpy(msg.user_name, clients[i].user_name);
                    strcpy(msg.queue_name, clients[i].queue_name);
                    //printf("Server1: (signal: %d, queue: %s, user: %s, buffer: %s)\n", msg.sign, msg.queue_name, msg.user_name, msg.buffer);
                    if (mq_send(clients[current].qd, (const char *)&msg, MAX_MSG_SIZE, 0) == -1)
                    {
                        perror("Server: Not able to send message to client");
                    }
                }
            }
            // отправка ранее отправленных сообщений
            //printf("Server: MSG66666: %d: %d %d\n", count_buf, start_buf, cur_buf);
            int i = start_buf;
            for (int c = 0; c < count_buf; c++)
            {
                if(i == MAX_MSGS_COUNT)
                    i = 0;
                //printf("Server2: (signal: %d, queue: %s, user: %s, buffer: %s)\n", ring_buff_msgs[i].sign, ring_buff_msgs[i].queue_name, ring_buff_msgs[i].user_name, ring_buff_msgs[i].buffer);
                if (mq_send(clients[current].qd, (const char *)&ring_buff_msgs[i], MAX_MSG_SIZE, 0) == -1)
                {
                    perror("Server: Not able to send message to client");
                }
                i++;
            }

            // для всех
            // отправка о новом подключении
            mymsg.sign = INIT;
            for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
            {
                if (clients[i].enable)
                {
                    //printf("Server3: (signal: %d, queue: %s, user: %s, buffer: %s)\n", mymsg.sign, mymsg.queue_name, mymsg.user_name, mymsg.buffer);
                    if (mq_send(clients[i].qd, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
                    {
                        perror("Server: Not able to send message to client");
                    }
                }
            }
        }
        break;
        case MSG:
        {
            printf("Server: MSG: %s: %s\n", mymsg.user_name, mymsg.buffer);

            count_buf++;

            if (count_buf > MAX_MSGS_COUNT)
            {
                cur_buf = start_buf;
                start_buf++;
                if(start_buf + 1 > MAX_MSGS_COUNT)
                    start_buf = 0;
                count_buf = MAX_MSGS_COUNT;
            }
            else
            {
                cur_buf++;
            }

            memset(&ring_buff_msgs[cur_buf], 0, MAX_MSG_SIZE);
            memcpy(&ring_buff_msgs[cur_buf], &mymsg, MAX_MSG_SIZE);

            // отправка нового сообщения всем клиентам
            for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
            {
                if (clients[i].enable)
                {
                    if (mq_send(clients[i].qd, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
                    {
                        perror("Server: Not able to send message to client");
                    }
                }
            }
        }
        break;
        case EXIT:
        {
            printf("Server: EXIT: %s\n", mymsg.user_name);

            for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
            {
                if (clients[i].enable)
                {
                    // отключение клиента
                    if (strcmp(clients[i].user_name, mymsg.user_name) == 0)
                    {
                        clients[i].enable = false;
                        count_cli--;
                    }
                    else
                    {
                        // отправка информации о отключении клиента
                        //printf("Server4: (signal: %d, queue: %s, user: %s, buffer: %s)\n", mymsg.sign, mymsg.queue_name, mymsg.user_name, mymsg.buffer);
                        if (mq_send(clients[i].qd, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
                        {
                            perror("Server: Not able to send message to client");
                        }
                    }
                }
            }
        }
        break;
        default:
            break;
        }
    }
}