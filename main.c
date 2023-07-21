#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 15


void sig_winch(int signo) {
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*)&size);
	resizeterm(size.ws_row, size.ws_col);
}

int main(int argc, char** argv) {

	WINDOW* wndtxt;
	WINDOW* wndnav;

	WINDOW* subwndtxt;
	WINDOW* subwndnav1;
	WINDOW* subwndnav2;
	WINDOW* subwndnav3;

    int size;
    char path[100];
    char txt[2812];
    char change[2812];
    int x = 1, y = 1;
    int fd;
    int buffer;
    int index;

	initscr();
	signal(SIGWINCH, sig_winch);
	cbreak();
	curs_set(TRUE);
    noecho();
    keypad(stdscr, TRUE);
	refresh();

	wndtxt = newwin(30, 100, 2, 4);
	wndnav = newwin(3, 100, 32, 4);

	box(wndnav, '|', '-');
	box(wndtxt, '|', '-');

	subwndtxt = derwin(wndtxt, 29, 97, 1, 2);
	subwndnav1 = derwin(wndnav, 2, 33, 1, 1);
	subwndnav2 = derwin(wndnav, 2, 33, 1, 34);
	subwndnav3 = derwin(wndnav, 2, 32, 1, 67);

	wprintw(subwndtxt, "Wellcome to my file editor!\n");
	wprintw(subwndnav1, "Press Esc for exit\n");
	wprintw(subwndnav2, "Press F5 for save\n");
	wprintw(subwndnav3, "Press F2 for open file\n");


    while (TRUE) {

        wmove(subwndtxt, y, x);
        wrefresh(wndtxt);
        wrefresh(wndnav);
        wrefresh(subwndtxt);

        buffer = getch();
        

        switch (buffer) {

            case KEY_UP:
                if (y != 0) {
                    y--;
                }
                else
                    y = 27;
                wmove(subwndtxt, y, x);
                break;
            case KEY_DOWN:
                if (y != 27) {
                    y++;
                }
                else
                    y = 0;
                wmove(subwndtxt, y, x);
                break;
            case KEY_LEFT:
                if (x != 0) {
                    x--;
                }
                else
                    x = 96;
                wmove(subwndtxt, y, x);
                break;
            case KEY_RIGHT:
                if (x != 96) {
                    x++;
                }
                else
                    x = 0;
                wmove(subwndtxt, y, x);
                break;
            case 266: //F2
                wclear(subwndtxt);
                system("clear");
                wmove(subwndtxt, 1, 1);
                wprintw(subwndtxt, "Enter the path and name (/path/name.txt)\n");
                wmove(subwndtxt, 2, 1);
                echo();
                wrefresh(subwndtxt);
                wgetstr(subwndtxt, path);
                fd = open(path, O_RDWR);
                if (!fd) {
                    break;
                }
                read(fd, txt, sizeof(txt) * sizeof(char));

                noecho();
                wclear(subwndtxt);
                system("clear");
                wprintw(subwndtxt, txt);
                wrefresh(subwndtxt);

                break;
            case 27: //ESC
                delwin(subwndtxt);
                delwin(wndtxt);
                refresh();
                endwin();
                exit(EXIT_SUCCESS);
                
            case 269: //Save by F3 
                fsync(fd);
                lseek(fd, 0, SEEK_SET);
                size = strcspn(txt, "\0");
                write(fd, txt, size);
                break;
            default:
                if (buffer == 263) {
                    buffer = 32;
                    x--;
                    wmove(subwndtxt, y, x);
                    txt[y * 97 + x] = buffer;
                    wprintw(subwndtxt, &buffer);
                }
                else {
                    wmove(subwndtxt, y, x);
                    txt[y * 97 + x] = buffer;
                    wprintw(subwndtxt, &buffer);
                    x++;
                }
                break;
        }
    }
}
