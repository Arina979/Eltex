// чат с общей комнатой

#include "header.h"

#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <curses.h>

mqd_t qd_server;
mqd_t qd_client;
char user_name[MSG_SIZE];
char queue_name[QUEUE_SIZE];

struct client
{
    bool enable;
    char user_name[MSG_SIZE];
};

struct window
{
    WINDOW *wnd;

    WINDOW *subwnd;
    int lines;
    int cols;

    int start;
    int cur;
};
struct window w_clients = {0};
struct window w_msgs = {0};
struct window w_input = {0};

bool interrupt = false;

void close_client()
{
    interrupt = true;
}

void close_window(struct window *w)
{
    delwin(w->subwnd);
    delwin(w->wnd);
}

void refresh_window(struct window *w)
{
    wrefresh(w->subwnd);
    wrefresh(w->wnd);
}

void create_window(struct window *w, int lines, int cols, int x, int y, const char *front)
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);//цвет символа, цвет фона
    w->wnd = newwin(lines, cols, x, y);//новое окно, строк, столбцов, верхний угол
    wattron(w->wnd, COLOR_PAIR(1));//устанавливает цвет фона и символов
    box(w->wnd, '|', '-');
    w->lines = lines - 2;
    w->cols = cols - 2;
    w->subwnd = derwin(w->wnd, w->lines, w->cols, 1, 1);
    wbkgd(w->subwnd, COLOR_PAIR(1));

    mvwprintw(w->subwnd, 0, 1, "%s", front);

    w->start = 2;
    w->cur = w->start;

    refresh_window(w);
}

// поток обработки сообщений от сервера
void* show_msgs(void *param)
{
    struct client clients[MAX_CLIENTS_COUNT];
    for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
        clients[i].enable = false;

    struct msg recv_msg = {0};
    struct timespec ts;
    while(!interrupt) 
    {
        memset(&recv_msg, 0, sizeof(struct msg));
        // получить самое старое сообщение с наивысшим приоритетом
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1; // Set a 1-second timeout
        if(mq_timedreceive(qd_client, (char *)&recv_msg, MSG_BUFFER_SIZE, NULL, &ts) == -1)
        //if (mq_receive(qd_client, (char *)&recv_msg, MSG_BUFFER_SIZE, NULL) == -1)
        {
            if (errno != ETIMEDOUT)
            {
                perror("Client: mq_receive");
                exit(1);
            }
            else
            {
                continue;
            }
            perror("Client: mq_receive");
            continue;
        }
        //printf("Client: message received (signal: %d, queue: %s, user: %s, buffer: %s)\n", recv_msg.sign, recv_msg.queue_name, recv_msg.user_name, recv_msg.buffer);

        switch(recv_msg.sign)
        {
            case INIT:
                {
                    wmove(w_clients.subwnd, w_clients.cur, 0);
                    wclrtoeol(w_clients.subwnd);

                    mvwprintw(w_clients.subwnd, w_clients.cur, 1, "%s", recv_msg.user_name);
                    w_clients.cur++;
                    if(w_clients.cur == w_clients.lines)
                        w_clients.cur = w_clients.start;
                    refresh_window(&w_clients);

                    for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
                    {
                        if (!clients[i].enable)
                        {
                            strcpy(clients[i].user_name, recv_msg.user_name);
                            clients[i].enable = true;
                            break;
                        }
                    }
                    //printf("Client: INIT: user: %s\n", recv_msg.user_name);
                }
                break;
            case MSG:
                {
                    wmove(w_msgs.subwnd, w_msgs.cur, 0);
                    wclrtoeol(w_msgs.subwnd);

                    mvwprintw(w_msgs.subwnd, w_msgs.cur, 1, "%s: %s", recv_msg.user_name, recv_msg.buffer);
                    w_msgs.cur++;
                    if(w_msgs.cur == w_msgs.lines)
                        w_msgs.cur = w_msgs.start;
                    refresh_window(&w_msgs);
                    //printf("Client: MSG: user: %s, buffer: %s", recv_msg.user_name, recv_msg.buffer);
                }
                break;
            case EXIT:
                {
                    wclear(w_clients.subwnd);
                    mvwprintw(w_clients.subwnd, 0, 1, "%s", "Clients");
                    refresh_window(&w_clients);
                    w_clients.cur = w_clients.start;

                    for (int i = 0; i < MAX_CLIENTS_COUNT; i++)
                    {
                        if (clients[i].enable)
                        {
                            if(strcmp(recv_msg.user_name, clients[i].user_name) == 0)
                            {
                                clients[i].enable = false;
                            }
                            else
                            {
                                // выводим весь список заново
                                wmove(w_clients.subwnd, w_clients.cur, 0);
                                wclrtoeol(w_clients.subwnd);

                                mvwprintw(w_clients.subwnd, w_clients.cur, 1, "%s", clients[i].user_name);
                                w_clients.cur++;
                                if(w_clients.cur == w_clients.lines)
                                    w_clients.cur = w_clients.start;
                            }
                        }
                    }
                    //printf("Client: EXIT: user: %s\n", recv_msg.user_name);
                }
                refresh_window(&w_clients);
                break;
            case EXIT_SERVER:
                {
                    //printf("Client: EXIT_SERVER\n");
                    close_client();
                }
                break;
            default:
                break;
        }
    }
}

int main (int argc, char **argv)
{
    if (signal(SIGINT, close_client) == SIG_ERR)
    {
        perror("Client: ERROR opening signal");
        exit(1);
    }

    if(argc != 2)
    {
        printf("Client: pass the client's name\n");
        exit(1);
    }
    else
    {
        sprintf(queue_name, "/my_client-%d", getpid());
        strcpy(user_name, argv[1]);
    }

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // создаем клиентскую очередь для получения сообщений от сервера;
    mq_unlink(queue_name);
    if ((qd_client = mq_open(queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1)
    {
        perror ("Client: mq_open");
        exit (1);
    }
    printf("Client: mq_open\n");

    // подключаемся к очереди сервера
    if ((qd_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit(1);
    }
    printf("Client: mq_open (server)\n");

    // INIT
    struct msg send_msg = {0};
    strcpy(send_msg.user_name, user_name);
    strcpy(send_msg.queue_name, queue_name);
    send_msg.sign = INIT;
    if (mq_send (qd_server, (const char *)&send_msg, MAX_MSG_SIZE, 0) == -1)
    {
        perror ("Client: Not able to send message to server");
        exit(1);
    }
    printf("Client: INIT\n");

    // ncurses
    initscr();//инициализация ncurses
    cbreak();//не дожидаясь нажатия [Enter], а клавиша [BackSpace] игнорируется
    start_color();//инициализирует управление цветом ncurses
    int middle = COLS / 3;
    create_window(&w_clients, LINES, middle, 0, 0, "Clients");
    create_window(&w_msgs, LINES, middle, 0, middle, "Msgs");
    create_window(&w_input, LINES, COLS - middle * 2, 0, middle * 2, "Input (enter e to exit)");
    wmove(w_input.subwnd, w_input.start, 1);
    curs_set(false);
    //keypad(w_input.subwnd, TRUE);
    wrefresh(w_input.subwnd);

    // поток обработки сообщений от сервера
    pthread_t th_msg;
    pthread_create(&th_msg, NULL, show_msgs, NULL);

    while(!interrupt) 
    {
        send_msg.sign = MSG;

        wgetnstr(w_input.subwnd, send_msg.buffer, MSG_SIZE);//ввод
        strcpy(send_msg.user_name, user_name);
        if (strcmp(send_msg.buffer, "e") == 0)
        {
            close_client();
            break;
        }
        wclear(w_input.subwnd);
        mvwprintw(w_input.subwnd, 0, 1, "%s", "Input (enter e to exit)");
        wmove(w_input.subwnd, w_input.start, 1);
        refresh_window(&w_input);

        // отправить сообщение на сервер
        if (mq_send (qd_server, (const char *)&send_msg, sizeof(struct msg), 0) == -1)
        {
            perror ("Client: Not able to send message to server");
            continue;
        }
    }

    struct msg mymsg = {0};
    strcpy(mymsg.user_name, user_name);
    strcpy(mymsg.queue_name, queue_name);
    mymsg.sign = EXIT;
    if (mq_send (qd_server, (const char *)&mymsg, MAX_MSG_SIZE, 0) == -1)
    {
        perror ("Client: Not able to send message to server");
    }

    if (mq_close (qd_client) == -1)
    {
        perror ("Client: mq_close");
    }

    if (mq_unlink (queue_name) == -1)
    {
        perror ("Client: mq_unlink");
    }
    printf ("Client: bye\n");

    pthread_join(th_msg, NULL);

    close_window(&w_clients);
    close_window(&w_msgs);
    close_window(&w_input);
    endwin();

    exit(0);
}