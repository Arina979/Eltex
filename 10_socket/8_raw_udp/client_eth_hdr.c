// передача сообщений между процессами через сокеты по сети по протоколу udp
// используется raw сокет
// заголовок транспортного уровня - udp заполняется самостоятельно
// заголовок сетевого уровня - ip заполняется самостоятельно
// заголовок канального уровня - ethernet заполняется самостоятельно

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
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>

#define S_IP   "192.168.1.10"
#define D_IP   "192.168.1.8"
#define S_PORT  8081
#define D_PORT  8080
#define MAXLINE 256

struct package
{
    struct ethhdr eh;
    struct iphdr ip;
    struct udphdr udp;
    char msg[MAXLINE];
} __attribute__((package)); // структура была без выравнивания

unsigned short sheck_sum(struct iphdr iph)
{
    unsigned int sum = 0;
    unsigned short *ptr = (unsigned short *)&iph;
    iph.check = 0; // убедитесь, что поле контрольной суммы равно нулю перед расчетом
    for (int i = 0; i < sizeof(struct iphdr)/2; i++)
    {
        sum += *ptr++;
    }
    // добавить перенос
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    return (unsigned short)(~sum);
}
  
int main() 
{
    // создание сокета
    // PF_PACKET - прямой доступ к пакетам на канальном уровне
    int server_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // ETH_P_ALL - получить все протоколы на канальном уровне
    if (server_fd < 0)
    {
        perror("client: ERROR opening socket");
        exit(1);
    }

    // индекс сетевого инткрфейса
    unsigned int ifindex = if_nametoindex("enp0s3");

    // подключиться к серверу
    struct sockaddr_ll serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_ll));
    serv_addr.sll_family = AF_PACKET;
    serv_addr.sll_ifindex = ifindex;
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_ll)) == -1)
    {
        perror("client: ERROR on bind");
        exit(1);
    }

    struct package pack;

    //ethernet заголовок
    pack.eh.h_source[0] = 0x08;
    pack.eh.h_source[1] = 0x00;
    pack.eh.h_source[2] = 0x27;
    pack.eh.h_source[3] = 0x72;
    pack.eh.h_source[4] = 0xb0;
    pack.eh.h_source[5] = 0x0c;
    pack.eh.h_dest[0] = 0xdc;
    pack.eh.h_dest[1] = 0x21;
    pack.eh.h_dest[2] = 0x48;
    pack.eh.h_dest[3] = 0x42;
    pack.eh.h_dest[4] = 0xad;
    pack.eh.h_dest[5] = 0xf6;
    pack.eh.h_proto = htons(ETH_P_IP);

    // ip заголовок
    pack.ip.version = 4; // Ipv4
    pack.ip.ihl = 5; // Длина заголовка - 5 означает, что в заголовке IP не отправляются дополнительные параметры
    pack.ip.tos = 0; // Тип обслуживания, не задаем
    pack.ip.tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + MAXLINE); // Общая длина пакета
    pack.ip.id = 0; // Идентификация фрагментов. Заполняется системой, когда = 0
    pack.ip.frag_off = 0; // Фрагментация IP
    pack.ip.ttl = 64; // Время жизни
    pack.ip.saddr = inet_addr(S_IP);
    pack.ip.daddr = inet_addr(D_IP);
    pack.ip.protocol = IPPROTO_UDP;
    pack.ip.check = sheck_sum(pack.ip);

    printf("client: checksum: %d\n", pack.ip.check);

    // udp заголовок
    pack.udp.source = htons(S_PORT);
    pack.udp.dest = htons(D_PORT);
    pack.udp.len = htons(sizeof(struct udphdr) + MAXLINE);
    pack.udp.check = 0;

    // сообщение
    printf("client: Please enter the message: ");
    fgets(pack.msg, MAXLINE - 1, stdin);

    // отправить сообщение
    int n = sendto(server_fd, (const char *)&pack, sizeof(struct package), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (n < 0)
    {
        perror("client: ERROR on sendto");
        close(server_fd);
        exit(1);
    }
    printf("client: Message sent successfully: %s", pack.msg);

    // получить сообщение
    do
    {
        memset(&pack, 0, sizeof(struct package));
        int n = recvfrom(server_fd, (char *)&pack, sizeof(struct package), 0, 0, 0);
        if (n < 0)
        {
            perror("client: ERROR on recvfrom");
            close(server_fd);
            exit(1);
        }
    } while (ntohs(pack.udp.dest) != S_PORT); 

    printf("client: Message received successfully: %s", pack.msg);
  
    close(server_fd); 
    return 0; 
}

//scp -P 3022 -r /home/arina/Eltex ubuntu@localhost:/home/ubuntu