// передача сообщений между процессами через сокеты по сети по протоколу tcp

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

int main()
{
    if (signal(SIGINT, close_fd) == SIG_ERR)
    {
        perror("server: ERROR opening signal");
        exit(1);
    }

    // создание сокета
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("client: ERROR opening socket");
        exit(1);
    }

    // подключиться к серверу
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("client: ERROR connecting");
        close(server_fd);
        exit(1);
    }

    char buffer[MAXLINE];
    while (1)
    {
        // отправить сообщение
        memset(&buffer, 0, MAXLINE);
        printf("client: Please enter the message: ");
        fgets(buffer, MAXLINE - 1, stdin);
        int n = send(server_fd, buffer, strlen(buffer), 0);
        if (n < 0)
        {
            perror("client: ERROR on send");
            close(server_fd);
            exit(1);
        }
        printf("client: Message sent successfully: %s", buffer);

        // получить сообщение
        memset(&buffer, 0, sizeof(buffer));
        n = recv(server_fd, buffer, MAXLINE, 0);
        if (n < 0)
        {
            perror("client: ERROR on recv");
            close(server_fd);
            exit(1);
        }
        printf("client: Message received successfully: %s", buffer);
    }

    close(server_fd);
    exit(0);
}