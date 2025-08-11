// передача сообщений между процессами через сокеты на одном ПК по протоколу tcp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "tcp_stream_serv"
#define MAXLINE 256

int main()
{
    // создание сокета
    int server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("server: ERROR opening socket");
        exit(1);
    }

    // удаление файла
    unlink(PATH);

    // привязать сокет к ip и port
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strcpy (serv_addr.sun_path, PATH);
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

    // ожидание подключения клиента
    struct sockaddr_un cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
    if (client_fd < 0)
    {
        perror("server: ERROR on accept");
        close(client_fd);
        close(server_fd);
        exit(1);
    }

    // получить сообщение
    char buffer[MAXLINE];
    memset(&buffer, 0, MAXLINE);
    if(recv(client_fd, buffer, MAXLINE, 0) < 0)
    {
        perror("server: ERROR on recv");
        close(client_fd);
        close(server_fd);
        unlink(PATH);
        exit(1);
    }
    printf("server: Message received successfully: %s", buffer) ;

    // отправить сообщение
    memset(&buffer, 0, MAXLINE);
    printf("server: Please enter the message: ");
    fgets(buffer, MAXLINE - 1, stdin);
    if(send(client_fd, buffer, MAXLINE, 0) < 0)
    {
        perror("server: ERROR on send");
        close(client_fd);
        close(server_fd);
        unlink(PATH);
        exit(1);
    }
    printf("server: Message sent successfully: %s", buffer);

    // закрытие файлового дескриптора
    close(client_fd);
    close(server_fd);
    // удаление файла
    unlink(PATH);
    return 0;
}
