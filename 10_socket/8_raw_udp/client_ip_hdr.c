// передача сообщений между процессами через сокеты по сети по протоколу udp
// используется raw сокет
// заголовок транспортного уровня - udp заполняется самостоятельно
// заголовок сетевого уровня - ip заполняется самостоятельно

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <linux/udp.h>
#include <linux/ip.h>

#define S_IP   "192.168.1.10"
#define D_IP   "192.168.1.8"
#define S_PORT  8081
#define D_PORT  8080
#define MAXLINE 256

struct package
{
    struct iphdr ip;
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

    // добавлять пользовательский заголовок IP
    int optval = 1;
    if (setsockopt(server_fd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0)
    {
        perror("client: ERROR setsockopt");
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

    struct package pack;

    // ip заголовок
    pack.ip.version = 4; // Ipv4
    pack.ip.ihl = 5; // Длина заголовка - 5 означает, что в заголовке IP не отправляются дополнительные параметры
    pack.ip.tos = 0; // Тип обслуживания, не задаем
    pack.ip.tot_len = 0; // Общая длина пакета. Заполняется системой
    pack.ip.id = 0; // Идентификация фрагментов. Заполняется системой, когда = 0
    pack.ip.frag_off = 0; // Фрагментация IP
    pack.ip.ttl = 64; // Время жизни
    pack.ip.protocol = IPPROTO_UDP;
    pack.ip.check = 0; // Заполняется системой
    pack.ip.saddr = inet_addr(S_IP);
    pack.ip.daddr = inet_addr(D_IP);

    // udp заголовок
    pack.udp.source = htons(S_PORT);
    pack.udp.dest = htons(D_PORT);
    pack.udp.len = htons(sizeof(struct package) - sizeof(struct iphdr));
    pack.udp.check = htons(0);

    // сообщение
    printf("client: Please enter the message: ");
    fgets(pack.msg, MAXLINE - 1, stdin);

    // отправить сообщение
    int n = send(server_fd, (const void *)&pack, sizeof(struct package), 0);
    if (n < 0)
    {
        perror("client: ERROR on send");
        close(server_fd);
        exit(1);
    }
    printf("client: Message sent successfully: %s", pack.msg);

    // получить сообщение
    do
    {
        memset(&pack, 0, sizeof(struct package));
        n = recv(server_fd, (void *)&pack, sizeof(struct package), 0);
        if (n < 0)
        {
            perror("client: ERROR on recv");
            close(server_fd);
            exit(1);
        }
    } while (ntohs(pack.udp.dest) != S_PORT);

    printf("client: Message received successfully: %s", pack.msg);
  
    close(server_fd); 
    return 0; 
}