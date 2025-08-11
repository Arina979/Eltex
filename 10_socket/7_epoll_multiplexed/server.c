// передача сообщений между процессами через сокеты по сети по протоколам tcp и udp
// сервер отслеживает нескольких открытых файловых дескрипторов(сокетов) с помощью epoll

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#define IP "127.0.0.1"
#define PORT 8080
#define MAXLINE 256
#define MAXEVENTS 64

int sfd_tcp;
int sfd_udp;
struct epoll_event *events;

static int make_socket_non_blocking(int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("server: ERROR on fcntl1");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror("server: ERROR on fcntl2");
        return -1;
    }

    return 0;
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

void close_fd()
{
    free(events);
    close(sfd_tcp);
    close(sfd_udp);
    exit(0);
}

int main()
{
    if (signal(SIGINT, close_fd) == SIG_ERR)
    {
        perror("server: ERROR opening signal");
        exit(1);
    }

    int efd;
    struct epoll_event event;

    // TCP
    // создание сокета
    sfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd_tcp < 0)
    {
        perror("server: ERROR opening socket1");
        exit(1);
    }
    // привязать сокет к ip и port
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);
    if (bind(sfd_tcp, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: ERROR on binding1");
        exit(1);
    }
    // для режима по фронту(EPOLLET) нужен неблокирующий дескриптор файла
    if (make_socket_non_blocking(sfd_tcp) < 0)
    {
        perror("server: ERROR on make_socket_non_blocking1");
        close(sfd_tcp);
        exit(1);
    }
    if (listen(sfd_tcp, SOMAXCONN) < 0)
    {
        perror("server: ERROR on listen");
        close(sfd_tcp);
        exit(1);
    }

    // UDP
    // создание сокета
    sfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd_udp < 0)
    {
        perror("server: ERROR opening socket2");
        close(sfd_tcp);
        exit(1);
    }
    // привязать сокет к ip и port
    if (bind(sfd_udp, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("server: ERROR on binding2");
        close(sfd_tcp);
        exit(1);
    }
    // установить время блокировки recv
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(sfd_udp, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    // epoll
    efd = epoll_create1(0);
    if (efd < 0)
    {
        perror("server: ERROR on epoll_create");
        close(sfd_tcp);
        close(sfd_udp);
        exit(1);
    }

    // TCP
    event.data.fd = sfd_tcp;
    // EPOLLIN — новые данные (для чтения) в файловом дескрипторе
    // EPOLLET - режим по фронту
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd_tcp, &event) < 0)
    {
        perror("server: ERROR on epoll_ctl1");
        close(sfd_tcp);
        close(sfd_udp);
        exit(1);
    }

    // UDP
    event.data.fd = sfd_udp;
    // EPOLLIN — новые данные (для чтения) в файловом дескрипторе
    // EPOLLET - режим по фронту
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd_udp, &event) < 0)
    {
        perror("server: ERROR on epoll_ctl2");
        close(sfd_tcp);
        close(sfd_udp);
        exit(1);
    }

    // буфер, куда возвращаются события
    events = calloc(MAXEVENTS, sizeof event);

    char buffer[MAXLINE];
    // цикл обработки событий
    while (1)
    {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) || // EPOLLERR — в файловом дескрипторе произошла ошибка
                (events[i].events & EPOLLHUP) || // EPOLLHUP — закрытие файлового дескриптора
                (!(events[i].events & EPOLLIN))) // !EPOLLIN — нет новых данных (для чтения) в файловом дескрипторе
            {
                // На этом fd произошла ошибка, или сокет не готов к чтению
                fprintf(stderr, "server: ERROR epoll\n");
                close(events[i].data.fd);
                continue;
            }
            else if (sfd_tcp == events[i].data.fd)
            {
                // TCP
                // На прослушиваемом сокете есть уведомление, что означает одно или несколько входящих соединений
                while (1)
                {
                    struct sockaddr_in cli_addr;
                    socklen_t clilen = sizeof(cli_addr);
                    int infd = accept(sfd_tcp, (struct sockaddr *)&cli_addr, &clilen);
                    if (infd < 0)
                    {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK))
                        {
                            // Мы обработали все входящие соединения
                            break;
                        }
                        else
                        {
                            perror("server: ERROR on accept");
                            break;
                        }
                    }

                    // Делаем входящий сокет неблокируемым и добавляем его в список для мониторинга
                    if (make_socket_non_blocking(infd) < 0)
                    {
                        perror("server: ERROR on make_socket_non_blocking2");
                        close_fd();
                    }

                    event.data.fd = infd;
                    // EPOLLIN — новые данные (для чтения) в файловом дескрипторе
                    // EPOLLET - режим по фронту
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event) < 0)
                    {
                        perror("server: ERROR on epoll_ctl3");
                        close_fd();
                    }
                }
                continue;
            }
            else if (sfd_udp == events[i].data.fd)
            {
                struct sockaddr_in cli_addr;
                socklen_t len = sizeof(cli_addr);
                while (1)
                {
                    // получить сообщение
                    memset(&buffer, 0, MAXLINE);
                    int n = recvfrom(sfd_udp, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cli_addr, &len);
                    if (n < 0)
                    {
                        break;
                    }
                    printf("server: Message received successfully: %s", buffer);

                    strrev(buffer); // перевернуть строку

                    // отправить сообщение
                    n = sendto(sfd_udp, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&cli_addr, len);
                    if (n < 0)
                    {
                        perror("server: ERROR on sendto");
                        break;
                    }
                    printf("server: Message sent successfully: %s", buffer);
                }
            }
            else
            {
                /* У нас есть данные на fd, ожидающие чтения. Читаем и
                 отобразить его. Мы должны прочитать все доступные данные
                 полностью, так как мы работаем в режиме срабатывания по фронту
                 и больше не получу уведомление о тех же данных. */
                int done = 0;

                while (1)
                {
                    // получить сообщение
                    memset(&buffer, 0, MAXLINE);
                    ssize_t count = recv(events[i].data.fd, buffer, MAXLINE, 0);
                    if (count == -1)
                    {
                        /* Если errno == EAGAIN, это значит, что мы прочитали все
                         данные. Так что возвращаемся к основному циклу. */
                        if (errno != EAGAIN)
                        {
                            perror("server: ERROR on recv");
                            printf("\n");
                            done = 1;
                        }
                        break;
                    }
                    else if (count == 0)
                    {
                        // Конец файла. Сокет закрыл связь
                        done = 1;
                        break;
                    }

                    strrev(buffer); // перевернуть строку

                    // отправить сообщение
                    if (send(events[i].data.fd, buffer, MAXLINE, 0) < 0)
                    {
                        perror("server: ERROR on send");
                        printf("\n");
                        close_fd();
                    }
                    printf("server: Message sent successfully: %s", buffer);
                }

                if (done)
                {
                    printf("Closed connection on descriptor %d\n", events[i].data.fd);

                    /* Закрытие дескриптора заставит epoll удалить его
                     из набора дескрипторов, которые отслеживаются. */
                    close(events[i].data.fd);
                }
            }
        }
    }

    close_fd();
}

// на основе примера:
// https://web.archive.org/web/20120504033548/https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/