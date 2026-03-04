// передача сообщений между процессами через сокеты по сети по протоколу udp
// для обслуживания каждого клиента создается процесс

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

#define IP "127.0.0.1"
#define PORT 8080
#define MAXLINE 256

int server_fd;

void close_fd()
{
    close(server_fd);
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

int main()
{
    if (signal(SIGINT, close_fd) == SIG_ERR)
    {
        perror("server: ERROR opening signal");
        exit(1);
    }

    // создание сокета для слушивающего сервера 
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

    char buffer[MAXLINE];
    int count_clients = 0;
    while (1)
    {
        // получить приветственное сообщение
        memset(&buffer, 0, MAXLINE);
        struct sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);
        int n = recvfrom(server_fd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cli_addr, &len);
        if (n < 0)
        {
            perror("server: ERROR on recvfrom");
            close(server_fd);
            exit(1);
        }
        printf("server: Welcome message from the client\n");

        count_clients++;

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("server: ERROR on fork");
            close(server_fd);
            exit(1);
        }
        if (pid == 0) // потомок
        {
            int client_fd;

            // создание сокета для обслуживания клиента
            client_fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (client_fd < 0)
            {
                perror("server: ERROR opening socket client");
                exit(1);
            }

            int port = PORT + count_clients;

            // привязать сокет к ip и port
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(IP);
            addr.sin_port = htons(port);
            if (bind(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                perror("server: ERROR on binding");
                exit(1);
            }

            // отправить новый port
            n = sendto(server_fd, &port, sizeof(port), 0, (const struct sockaddr *)&cli_addr, len);
            if (n < 0)
            {
                perror("server: ERROR on sendto");
                close(client_fd);
                exit(1);
            }
            printf("server: Port sent successfully: %d\n", port);

            // закрытие файлового дескриптора
            close(server_fd);

            while (1)
            {
                // получить сообщение
                memset(&buffer, 0, MAXLINE);
                int n = recvfrom(client_fd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cli_addr, &len);
                if (n < 0)
                {
                    perror("server: ERROR on recvfrom");
                    close(client_fd);
                    exit(1);
                }
                printf("server: Message received successfully: %s", buffer);

                // перевернуть строку
                strrev(buffer); 

                // отправить сообщение
                n = sendto(client_fd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&cli_addr, len);
                if (n < 0)
                {
                    perror("server: ERROR on sendto");
                    close(client_fd);
                    exit(1);
                }
                printf("server: Message sent successfully: %s", buffer);
            }
            close(client_fd);
            exit(0);
        }
    }

    // закрытие файлового дескриптора
    close(server_fd);
    return 0;
}
