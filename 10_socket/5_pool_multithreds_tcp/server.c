// передача сообщений между процессами через сокеты по сети по протоколу tcp
// сразу создается пул из 5 обслуживающих серверов (5 потоков)
// при необходимости пул серверов расширяется

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

#define IP "127.0.0.1"
#define PORT 8080
#define MAXLINE 256
#define POOL 5

struct serving_server
{
    pthread_t pth;
    int client_fd;
    int num;
};

// слушающий сервер
int server_fd = 0;

// обслуживающие серверы
int n_clients = 0;
struct serving_server *serving_servers = NULL;
pthread_rwlock_t lock_rw = PTHREAD_RWLOCK_INITIALIZER;

void close_fd()
{
    close(server_fd);

    pthread_rwlock_wrlock(&lock_rw);
    for (int i = 0; i < n_clients; i++)
    {
        if (!serving_servers[i].pth)
            continue;
        pthread_cancel(serving_servers[i].pth);
        pthread_join(serving_servers[i].pth, 0);
        close(serving_servers[i].client_fd);
    }
    free(serving_servers);
    pthread_rwlock_unlock(&lock_rw);

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
    pthread_rwlock_rdlock(&lock_rw);
    int num = *(int *)args;
    int client_fd = serving_servers[num].client_fd;
    // printf("%d %d\n", num, client_fd);
    pthread_rwlock_unlock(&lock_rw);

    char buffer[MAXLINE];
    while (1)
    {
        if (client_fd)
        {
            // получить сообщение
            memset(&buffer, 0, MAXLINE);
            if (recv(client_fd, buffer, MAXLINE, 0) < 0)
            {
                perror("server: ERROR on recv");
                printf("%d %d\n", num, client_fd);
                close(client_fd);

                pthread_rwlock_wrlock(&lock_rw);
                serving_servers[num].client_fd = 0;
                client_fd = 0;
                pthread_rwlock_unlock(&lock_rw);

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

                pthread_rwlock_wrlock(&lock_rw);
                serving_servers[num].client_fd = 0;
                client_fd = 0;
                pthread_rwlock_unlock(&lock_rw);

                continue;
            }
            printf("server: Message sent successfully: %s", buffer);
        }
        else
        {
            pthread_rwlock_rdlock(&lock_rw);
            client_fd = serving_servers[num].client_fd;
            pthread_rwlock_unlock(&lock_rw);
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
        abort();
    }

    // создание poll обслуживающих серверов
    serving_servers = malloc(sizeof(struct serving_server) * POOL);
    if (serving_servers == 0)
    {
        perror("server: ERROR on malloc");
        exit(1);
    }
    n_clients = POOL;
    for (int i = 0; i < n_clients; i++)
    {
        serving_servers[i].pth = 0;
        serving_servers[i].client_fd = 0;
        serving_servers[i].num = i;
        pthread_create(&serving_servers[i].pth, NULL, &pthread_server, &serving_servers[i].num);
    }

    while (1)
    {
        // ожидание подключения клиента
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (fd < 0)
        {
            perror("server: ERROR on accept");
            close(fd);
            close(server_fd);
            exit(1);
        }

        // инициализация или расширение массива серверов
        bool new_server = true;
        pthread_rwlock_wrlock(&lock_rw);
        for (int i = 0; i < n_clients; i++)
        {
            if (serving_servers[i].client_fd == 0)
            {
                serving_servers[i].client_fd = fd;
                new_server = false;
                break;
            }
        }
        if (new_server)
        {
            n_clients++;
            serving_servers = realloc(serving_servers, sizeof(struct serving_server) * n_clients);
            if (serving_servers == 0)
            {
                perror("Realloc error");
                exit(1);
            }
            serving_servers[n_clients - 1].client_fd = fd;
            serving_servers[n_clients - 1].pth = 0;
            serving_servers[n_clients - 1].num = n_clients - 1;
            // создание потока
            int i = n_clients - 1;
            pthread_create(&serving_servers[n_clients - 1].pth, NULL, &pthread_server, &serving_servers[n_clients - 1].num);
        }
        pthread_rwlock_unlock(&lock_rw);
    }

    close_fd();
    return 0;
}