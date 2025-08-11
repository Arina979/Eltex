// передача сообщений между процессами через сокеты по сети по протоколу tcp
// сразу создается пул из 5 обслуживающих серверов (5 потоков)
// при поступлении запроса, сервер располагает заявку в очередь
// обслуживающий сервер забирает заявку, обрабатывает и отвечает клиенту

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <mqueue.h>

#define IP "127.0.0.1"
#define PORT 8080
#define MAXLINE 256
#define POOL 5

#define QUEUE_NAME "/queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(int)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)

int server_fd = 0;
int n_servers = 0;
pthread_t *pths;

mqd_t mqd;

void close_fd()
{
    close(server_fd);

    for (int i = 0; i < n_servers; i++)
    {
        if (!pths[i])
            continue;
        pthread_cancel(pths[i]);
        pthread_join(pths[i], 0);
    }
    free(pths);

    if (mq_close(mqd) == -1)
    {
        perror("server: ERROR on mq_close");
        exit(1);
    }
    if (mq_unlink(QUEUE_NAME) == -1)
    {
        perror ("server: ERROR on mq_unlink");
        exit (1);
    }

    exit(0);
}

void strrev(char *head)
{
    if (!*head)
        return;
    char *tail = head;
    while (*tail != '\n' && *tail)
        ++tail; // tail указывает на последний действительный char
    --tail;     // head по-прежнему указывает на первый

    for (; head < tail; ++head, --tail)
    {
        // идем по указателям внутрь, пока они не встретятся или не пересекутся посередине
        char h = *head, t = *tail;
        *head = t; // меняем местами по ходу
        *tail = h;
    }
}

void *pthread_server(void *args)
{
    char buffer[MAXLINE];

    int client_fd = 0;
    int num_read = 0;

    while (1)
    {
        if (client_fd)
        {
            // получить сообщение
            memset(&buffer, 0, MAXLINE);
            if (recv(client_fd, buffer, MAXLINE, 0) < 0)
            {
                perror("server: ERROR on recv");
                printf("\n");
                close(client_fd);
                client_fd = 0;
                continue;
            }
            printf("server: Message received successfully: %s", buffer);

            strrev(buffer); // перевернуть строку

            // отправить сообщение
            if (send(client_fd, buffer, MAXLINE, 0) < 0)
            {
                perror("server: ERROR on send");
                printf("\n");
                close(client_fd);
                client_fd = 0;
                continue;
            }
            printf("server: Message sent successfully: %s", buffer);
        }
        else
        {
            // получить дескриптор из очереди
            num_read = 0;
            num_read = mq_receive(mqd, (char *)&client_fd, MSG_BUFFER_SIZE, 0);
            if (num_read == -1)
            {
                perror("server: ERROR on mq_receive");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}

int main()
{
    if (signal(SIGINT, close_fd) == SIG_ERR)
    {
        perror("server: ERROR opening signal");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);

    // создание сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("server: ERROR opening socket");
        exit(1);
    }

    // привязать сокет к ip и port
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: ERROR on binding");
        exit(1);
    }

    // подготовка к приему подключений на сокете
    if(listen(server_fd, 5) < 0)
    {
        perror("server: ERROR on listen");
        close(server_fd);
        exit(1);
    }

    // создать очередь
    mq_unlink(QUEUE_NAME);
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    mqd = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mqd == (mqd_t)-1)
    {
        perror("server: ERROR on mq_open");
        close(server_fd);
        exit(1);
    }

    // создание poll обслуживающих серверов
    pths = malloc(sizeof(pthread_t) * POOL);
    if (pths == 0)
    {
        perror("server: ERROR on malloc");
        close(server_fd);
        exit(1);
    }
    n_servers = POOL;
    for (int i = 0; i < n_servers; i++)
    {
        pthread_create(&pths[i], NULL, &pthread_server, 0);
    }

    while (1)
    {
        // ожидание подключения клиента
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (client_fd < 0)
        {
            perror("server: ERROR on accept");
            close(client_fd);
            close(server_fd);
            exit(1);
        }

        // отправить сообщение в очередь
        if (mq_send(mqd, (const char *)&client_fd, sizeof(client_fd), 0) == -1)
        {
            perror("server: ERROR on mq_send");
            exit(1);
        }
    }

    close_fd();
    return 0;
}