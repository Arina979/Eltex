// передача сообщений между процессами через сокеты на одном ПК по протоколу tcp

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>

#define PATH "tcp_stream_serv"
#define MAXLINE 256

int main()
{
    // создание сокета
    int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("client: ERROR opening socket");
        exit(1);
    }

    // подключиться к серверу
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strcpy (serv_addr.sun_path, PATH);
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("client: ERROR connecting");
        close(server_fd);
        exit(1);
    }

    // отправить сообщение
    char buffer[MAXLINE];
    memset(&buffer, 0, MAXLINE);
    printf("client: Please enter the message: ");
    fgets(buffer, MAXLINE - 1, stdin);
    if(send(server_fd, buffer, MAXLINE, 0) < 0)
    {
        perror("client: ERROR on send");
        close(server_fd);
        exit(1);
    }
    printf("client: Message sent successfully: %s", buffer);

    // получить сообщение
    memset(&buffer, 0, MAXLINE);
    if(recv(server_fd, buffer, MAXLINE, 0) < 0)
    {
        perror("client: ERROR on recv");
        close(server_fd);
        exit(1);
    }
    printf("client: Message received successfully: %s", buffer);

    // закрытие файлового дескриптора
    close(server_fd);
    return 0;
}