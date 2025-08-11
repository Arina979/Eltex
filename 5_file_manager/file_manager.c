// file_manager.c - файловый менеджер с двумя окнами, наподобие mc
// Функционал:
// * отображает путь до текущего каталога
// * отображает содержимое текущего каталога (в формате: имя файла, последнее изменение, размер)
// * позволяет перемещаться по списку файлов вверх и вниз
// * позволяет перемещаться между двумя окнами вправо и влево
// * заходить в каталоги и выходить из них
// * Ctrl + C - выход

// gcc file_manager.c -o file_manager -lncurses

#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <curses.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdio.h> 
#include <unistd.h>
#include <libgen.h>

#define SIZE_NAME 256
#define SIZE_TIME 13
#define SIZE_LEN 6

struct line
{
    char name[SIZE_NAME];
    char time[SIZE_TIME];
    unsigned short len;
    bool is_dir;
};

struct window
{
    WINDOW *wnd;

    WINDOW *subwnd;
    int lines;
    int cols;

    WINDOW *subwnd_error;

    int n;
    struct line *l;

    char current_dir[SIZE_NAME];
};

// //сигнал SIGWINCH - изменении размеров окна терминала
// void sig_winch(int signo)
// {
//     struct winsize size;
//     ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
//     resizeterm(size.ws_row, size.ws_col);
// }

void refreshWindow(struct window *w)
{
    wrefresh(w->subwnd);
    wrefresh(w->wnd);
}

void createColumns(struct window *w)
{
    mvwprintw(w->subwnd, 1, 1, "Name");
    mvwprintw(w->subwnd, 1, w->cols - SIZE_TIME, "Edit time");
    wmove(w->subwnd, 1, w->cols - SIZE_TIME - 1);
    wvline(w->subwnd, '|', w->lines);
    mvwprintw(w->subwnd, 1, w->cols - SIZE_TIME - 1 - SIZE_LEN - 1, "Size");
    wmove(w->subwnd, 1, w->cols - SIZE_TIME - 1 - SIZE_LEN - 1 - 1);
    wvline(w->subwnd, '|', w->lines);
}

void createWindow(struct window *w, int lines, int cols, int x, int y)
{
    init_pair(1, COLOR_WHITE, COLOR_BLUE);//цвет символа, цвет фона
    w->wnd = newwin(lines, cols, x, y);//новое окно, строк, столбцов, верхний угол
    wattron(w->wnd, COLOR_PAIR(1));//устанавливает цвет фона и символов
    box(w->wnd, '|', '-');
    w->lines = lines - 2;
    w->cols = cols - 2;
    w->subwnd = derwin(w->wnd, w->lines, w->cols, 1, 1);
    wbkgd(w->subwnd, COLOR_PAIR(1));

    w->subwnd_error = derwin(w->wnd, 1, cols, lines - 1, 0);
    //wprintw(w->subwnd_error, "Hello");

    createColumns(w);

    //текущий каталог
    char cwd[SIZE_NAME] = {0};
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        mvwprintw(w->subwnd, 0, 1, "%s", cwd);
        strcpy(w->current_dir, cwd);
    }
    else
    {
        printw("Getcwd error\n");
        return;
    }

    refreshWindow(w);

    w->n = 0;
    w->l = NULL;
}

void closeWindow(struct window *w)
{
    delwin(w->subwnd);
    delwin(w->wnd);
    delwin(w->subwnd_error);
    free(w->l);
}

void openDir(struct window *w, char * dir)
{
    //вывод текущей директории
    if(strcmp(dir, ".") == 0)
    {
        mvwprintw(w->subwnd, 0, 1, "%s", w->current_dir);
    }
    else if(strcmp(dir, "..") == 0)
    {
        dirname(w->current_dir);
        mvwprintw(w->subwnd, 0, 1, "%s", w->current_dir);
    }
    else
    {
        if(w->current_dir[1] != '\0')//корневой файл /
        {
            strcat(w->current_dir, dir);
        }
        else
        {
            strcpy(w->current_dir, dir);
        }
        mvwprintw(w->subwnd, 0, 1, "%s", w->current_dir);
    }

    //поиск файлов в директории
    struct dirent **namelist;
    int n = scandir(w->current_dir, &namelist, NULL, alphasort);
    if (n < 0)
    {
        wprintw(w->subwnd_error, "Scandir error");
        return;
    }

    //выделение памяти под текщий список файлов
    if(w->n == 0)
    {
        w->n = n - 1;
        w->l = malloc(w->n * sizeof(struct line));
        if(w->l == NULL)
        {
            wprintw(w->subwnd_error, "Malloc error");
            return;
        }
    }
    else
    {
        w->n = n - 1;
        w->l = realloc(w->l, w->n * sizeof(struct line));
        if(w->l == NULL)
        {
            wprintw(w->subwnd_error, "Realloc error");
            return;
        }
        memset(w->l, 0, w->n * sizeof(struct line));
    }

    for (int i = 0; i < w->n; i++)
    {
        char fullname[SIZE_NAME];
        strcpy(fullname, w->current_dir);
        strcat(fullname, "/");
        strcat(fullname, namelist[i + 1]->d_name);
        
        //время
        struct stat attr;
        if (stat(fullname, &attr) != 0)
        {
            wprintw(w->subwnd_error, "Stat error");
            return;
        }
        struct tm *t_tm = localtime(&attr.st_mtime);
        char time[SIZE_TIME] = {0};
        if (t_tm->tm_year + 1900 == 2025)
            strftime(time, SIZE_TIME, "%b %d %H:%M\n", t_tm);
        else
            strftime(time, SIZE_TIME, "%b %d  %Y\n", t_tm);

        //заполнение
        strcpy(w->l[i].name, namelist[i + 1]->d_name);
        strcpy(w->l[i].time, time);
        w->l[i].len = attr.st_size;
        w->l[i].is_dir = S_ISDIR(attr.st_mode);
        if(i != 0 && w->l[i].is_dir)
        {
            char *tmp = strdup(w->l[i].name);
            strcpy(w->l[i].name, "/");
            strcat(w->l[i].name, tmp);
            free(tmp);
        }
        //вывод
        mvwprintw(w->subwnd, i+2, 1, "%s ", w->l[i].name);
        mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME, "%-*s", SIZE_TIME, w->l[i].time);
        if(i == 0)
            mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*s", SIZE_LEN, "UP");
        else
            mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*d", SIZE_LEN, w->l[i].len);
        //mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*d", SIZE_LEN, w->l[i].is_dir);

        free(namelist[i+1]);
    }

    free(namelist[0]);
    free(namelist);
    refreshWindow(w);
}

int main(int argc, char ** argv)
{
    struct window w1 = {0};
    struct window w2 = {0};

    initscr();//инициализация ncurses
    //signal(SIGWINCH, sig_winch);
    cbreak();//не дожидаясь нажатия [Enter], а клавиша [BackSpace] игнорируется
    curs_set(FALSE);
    start_color();//инициализирует управление цветом ncurses
    init_pair(1, COLOR_WHITE, COLOR_BLUE);//цвет символа, цвет фона

    int middle = COLS / 2;
    createWindow(&w1, LINES, middle, 0, 0);
    createWindow(&w2, LINES, COLS - middle, 0, middle);
    int lines = LINES - 5;

    int choice;
    int highlight=0;//выбранная строка
    struct window *w = &w1;

    noecho(); //выключаем отображение вводимых символов

    openDir(&w1, ".");
    openDir(&w2, ".");

    keypad(w->subwnd, TRUE);//включаем работу с клавиатурой в окне
    int start = 0;//номер строки в списке, с которой начинать выводить
    int page = 1;//страница
    while(1)//цикл используется для непрерывного создания меню с выделенным выбранным элементом
    {
        int c_start = start;
        for(int i = 0; i < w->n && c_start < w->n; i++)
        {
            if(i == highlight)
                wattron(w->subwnd, A_REVERSE);

            mvwprintw(w->subwnd, i+2, 1, "%s ", w->l[c_start].name);
            mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME, "%-*s", SIZE_TIME, w->l[c_start].time);
            if(i == 0)
                mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*s", SIZE_LEN, "UP");
            else
                mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*d", SIZE_LEN, w->l[i].len);
            //mvwprintw(w->subwnd, i+2, w->cols - SIZE_TIME - SIZE_LEN - 1, "%*d", SIZE_LEN, w->l[c_start].is_dir);

            if (i == highlight)
                wattroff(w->subwnd, A_REVERSE);

            c_start++;
        }

        choice = wgetch(w->subwnd);
        switch(choice)
        {
        case KEY_UP:
            highlight--;
            if(page == 1 && highlight < 0)
            {
                highlight = 0;//защита от выхода "вне" меню
                break;
            }
            if(highlight < 0)//загрузить предыдущую страницу
            {
                wclear(w->subwnd);
                createColumns(w);
                mvwprintw(w->subwnd, 0, 1, "%s", w->current_dir);
                page--;
                start = lines * (page - 1);
                highlight = lines;
            }
            break;
        case KEY_DOWN:
            highlight++;
            if((page - 1) * (lines + 1) + highlight > (w->n - 1))
            {
                highlight--;//защита от выхода "вне" меню
                break;
            }
            if(highlight > lines)//загрузить следующую страницу
            {
                wclear(w->subwnd);
                createColumns(w);
                mvwprintw(w->subwnd, 0, 1, "%s", w->current_dir);
                start = highlight * page;
                page++;
                highlight = 0;
            }
            break;
        case KEY_LEFT:
            if(w == &w2)//переключиться на меню слева
            {
                start = 0;
                page = 1;
                w = &w1;
                keypad(w->subwnd, TRUE);
            }
            break;
        case KEY_RIGHT://переключиться на меню справа
            if(w == &w1)
            {
                start = 0;
                page = 1;
                w = &w2;
                keypad(w->subwnd, TRUE);
            }
            break;
        case 10://enter
            if(w->l[highlight].is_dir)
            {
                wclear(w->subwnd);
                createColumns(w);
                openDir(w, w->l[highlight].name);
                highlight = 0;
                start = 0;
                page = 1;
            }
            break;
        case 27://esc, выход
            closeWindow(&w1);
            closeWindow(&w2);
            endwin();//завершение ncurses
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    closeWindow(&w1);
    closeWindow(&w2);
    endwin();//завершение ncurses
    exit(EXIT_SUCCESS);
}


// лишний / +
// отменить измение размера, задать изначально
// пролистать вниз +
// пролистать вверх +
// неверно определяются дирректории +
// ошибки +
// setlocate rus
// значение не влазит в поле, обрезать
// утечки +