// передача сообщений между процессами через сокеты по сети по протоколу udp
// используется raw сокет
// заголовок транспортного уровня - udp заполняется самостоятельно

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <linux/udp.h>

#define D_IP   "192.168.1.8"
#define S_PORT  8081
#define D_PORT  8080
#define MAXLINE 256

struct send_package
{
    struct udphdr udp;
    char msg[MAXLINE];
};

struct recv_package
{
    char network[20];
    struct udphdr udp;
    char msg[MAXLINE];
};
  
int main() 
{
    // создание сокета
    int server_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (server_fd < 0)
    {
        perror("client: ERROR opening socket");
        exit(1);
    }

    // подключиться к серверу
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(D_IP);
    serv_addr.sin_port = htons(D_PORT); // endpoint не создается
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("client: ERROR connecting");
        exit(1);
    }

    struct send_package s_pack;
    // udp заголовок
    s_pack.udp.source = htons(S_PORT);
    s_pack.udp.dest = htons(D_PORT);
    s_pack.udp.len = htons(sizeof(struct send_package));
    s_pack.udp.check = htons(0);

    // сообщение
    printf("client: Please enter the message: ");
    fgets(s_pack.msg, MAXLINE - 1, stdin);

    // отправить сообщение
    int n = send(server_fd, (const void *)&s_pack, sizeof(struct send_package), 0);
    if (n < 0)
    {
        perror("client: ERROR on send");
        close(server_fd);
        exit(1);
    }
    printf("client: Message sent successfully: %s", s_pack.msg);

    // получить сообщение
    struct recv_package r_pack;
    do
    {
        memset(&r_pack, 0, sizeof(struct recv_package));
        n = recv(server_fd, (void *)&r_pack, sizeof(struct recv_package), 0);
        if (n < 0)
        {
            perror("client: ERROR on recv");
            close(server_fd);
            exit(1);
        }
    } while (ntohs(r_pack.udp.dest) != S_PORT);
    
    printf("client: Message received successfully: %s", r_pack.msg);
  
    close(server_fd); 
    return 0; 
}