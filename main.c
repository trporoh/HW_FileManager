#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>


WINDOW* left;
WINDOW* right;

WINDOW* subwndl;
WINDOW* subwndr;

WINDOW* ptrright;
WINDOW* ptrleft;
WINDOW* ptr;

pid_t file_editor_pid;

int key;

char** wnd_print(DIR* dir, WINDOW* wnd, char** names, char* types, int* string); 
char** change(DIR** open, WINDOW* ptrtxt, WINDOW* ptrpath, char** ptrnames, char* types, char** buftmp, int ptrstr, int* string);
void editor(char* buf, char* name);

int main(int args, char** argv) {

	initscr();
	curs_set(FALSE);
	noecho();
	cbreak();

	left = newwin(40, 70, 0, 0);
	right = newwin(40, 70, 0, 71);

	subwndl = derwin(left, 35, 60, 3, 5);
	subwndr = derwin(right, 35, 60, 3, 5);

	ptrleft = derwin(left, 35, 2, 3, 1);
	ptrright = derwin(right, 35, 2, 3, 1);

	ptr = ptrleft;

	keypad(stdscr, TRUE);
	refresh();

    char* bufl = malloc(128);
	char* bufr = malloc(128);
	char** namesl;
	char** namesr;

	int stringl = 0;
    int stringr = 0;
	int ptrstr = 0;
	char* typel = (char*)malloc(256 * sizeof(char));
	char* typer = (char*)malloc(256 * sizeof(char));
	int* ptr_type = typel;

	getcwd(bufl, 128);
	getcwd(bufr, 128);
	
	box(left, '|', '-');
	box(right, '|', '-');

	wmove(left, 1, 1);
	wprintw(left, bufl);

	wmove(right, 1, 1);
	wprintw(right, bufr);

	DIR* open = opendir(bufr);
	namesr = wnd_print(open, subwndr, namesr, typer, &stringr);
	open = opendir(bufr);
	namesl = wnd_print(open, subwndl, namesl, typel, &stringl);
	
	wrefresh(left);
	wrefresh(right);
	wrefresh(subwndl);
	wrefresh(subwndr);

	while (true) {
		wmove(ptr, ptrstr, 0);
		wprintw(ptr, "->\n");
        wrefresh(ptr);

        key = getch();


        switch (key) {

        case KEY_UP:
			wmove(ptr, ptrstr, 0);
			wprintw(ptr, "  ");
			if (ptrstr != 0) {
				ptrstr--;
			}
			else if (ptr == ptrleft) {
				ptrstr = stringl - 1;
			}
			else {
				ptrstr = stringr - 1;
			}
        break;
        case KEY_DOWN:
			wmove(ptr, ptrstr, 0);
			wprintw(ptr, "  ");
			if (ptr == ptrleft){
				if (ptrstr != stringl - 1) 
					ptrstr++;
				else
					ptrstr = 0;
			}
			else{
				if (ptrstr != stringr - 1) 
					ptrstr++;
				else
					ptrstr = 0;
				}
        break;
            case KEY_LEFT:
			case KEY_RIGHT:
				wmove(ptr, ptrstr, 0);
				wprintw(ptr, "  ");
				wrefresh(ptr);
				ptrstr = 0;
				if (ptr == ptrleft) {
					ptr = ptrright;
					ptr_type = typer;
				}
				else {
					ptr = ptrleft;
					ptr_type = typel;
				}
                break;
			case 10: //ENTER
				if (typel[ptrstr] != DT_DIR) {
					file_editor_pid = fork();
					if (!file_editor_pid) {
						editor(bufl, namesl[ptrstr]);
					}
					else {
						wait(NULL);
						system("clear");
						wrefresh(left);
						wrefresh(right);
						wrefresh(subwndl);
						wrefresh(subwndr);
						break;
					}
				}
					else if (ptr == ptrleft) {
						namesl = change(&open, subwndl, left, namesl, typel, &bufl, ptrstr, &stringl);
					}
					else{
						namesr = change(&open, subwndr, right, namesr, typer,  &bufr, ptrstr, &stringr);
					}
			
				break;
        case 27: //ESC
            delwin(left);
            delwin(right);
            refresh();
            endwin();
			system("clear");
            exit(EXIT_SUCCESS);
        }        
	}

	return 0;
}

char** wnd_print(DIR* dir, WINDOW* wnd, char** names, char* types, int* string) {

	names = (char**)malloc(256);
	struct dirent* data;

	int i = 0;

	while ((256 / sizeof(char*)) > i) {
		names[i] = malloc(32);
		i++;
	}
	i = 0;
	rewinddir(dir);
	while (((data = readdir(dir)) != NULL) && 30 > i) {
		wmove(wnd, i, 0);
		wprintw(wnd, data->d_name);
		if (data->d_type == DT_DIR) {
			wmove(wnd, i, 55);
			wprintw(wnd, "dir");
		}
		strcpy(&types[i], &(data->d_type));
		strcpy(names[i], data->d_name);
		i++;
	}
	names = (char**)realloc(names, sizeof(char*) * (i + 1));

	*string = i;

	return names;
}

char** change(DIR** open, WINDOW* ptrtxt, WINDOW* ptrpath, char** ptrnames, char* types, char** buftmp, int ptrstr, int* string) {

	rewinddir(*open);
	int size = 0;

	wclear(ptrtxt);
	wclear(ptrpath);
	if (!strncmp((ptrnames)[ptrstr], ".", 2)) {
		return ptrnames;
	}
	system("clear");
	if (!strncmp((ptrnames)[ptrstr], "..", 2)) {
		size = strlen(*buftmp) - 1;
		while ((*buftmp)[size] != '/') {
			size--;
		}
		(*buftmp) = (char*)realloc(*buftmp, sizeof(char) * (size + 1));
		(*buftmp)[size] = '\0';
	}
	else {
		(*buftmp) = strcat(*buftmp, "/");
		(*buftmp) = strcat(*buftmp, (ptrnames)[ptrstr]);
	}
	closedir(*open);
	*open = opendir(*buftmp);

	ptrnames = wnd_print(*open, ptrtxt, ptrnames, types, string);

	wmove(ptrpath, 1, 1);
	wprintw(ptrpath, *buftmp);
	box(ptrpath, '|', '-');

	wrefresh(ptrpath);
	wrefresh(ptrtxt);

	return ptrnames;
}

void editor(char* buf, char* name) {

	int fd;
	int x = 1, y = 1;
	char* path = malloc(strlen(buf) + strlen(name) + 1);
	char txt[2812];

	strcpy(path, buf);
	strcat(path, "/");
	strcat(path, name);
	system("clear");

	curs_set(TRUE);
	refresh();

	WINDOW* editor_window = newwin(30, 100, 2, 4);
	WINDOW* editor_text_subwindow = derwin(editor_window, 27, 97, 1, 1);

	fd = open(path, O_RDWR);
	if (!fd) {
		exit(EXIT_FAILURE);
	}
	read(fd, txt, sizeof(txt) * sizeof(char));

	noecho();
	wclear(editor_window);
	system("clear");
	wprintw(editor_text_subwindow, txt);
	wrefresh(editor_text_subwindow);
	box(editor_window, '|', '-');

	wmove(editor_window, 29, 1);
	wprintw(editor_window, "Press Esc for exit\n");
	wmove(editor_window, 29, 30);
	wprintw(editor_window, "Press F5 for save\n");


	while (TRUE) {

		wmove(editor_window, y, x);
		wrefresh(editor_window);

		key = getch();


		switch (key) {

		case KEY_UP:
			if (y != 0) {
				y--;
			}
			else
				y = 27;
			wmove(editor_text_subwindow, y, x);
			break;
		case KEY_DOWN:
			if (y != 27) {
				y++;
			}
			else
				y = 0;
			wmove(editor_text_subwindow, y, x);
			break;
		case KEY_LEFT:
			if (x != 0) {
				x--;
			}
			else
				x = 96;
			wmove(editor_text_subwindow, y, x);
			break;
		case KEY_RIGHT:
			if (x != 96) {
				x++;
			}
			else
				x = 0;
			wmove(editor_text_subwindow, y, x);
			break;
		case 27: //ESC
			delwin(editor_text_subwindow);
			delwin(editor_window);
			refresh();
			exit(EXIT_SUCCESS);

		case 269: //Save by F3 
			fsync(fd);
			lseek(fd, 0, SEEK_SET);
			write(fd, txt, strcspn(txt, "\0"));
			break;
		default:
			if (key == 263) {
				key = 32;
				x--;
				wmove(editor_text_subwindow, y, x);
				txt[y * 97 + x] = key;
				wprintw(editor_text_subwindow, &key);
			}
			else {
				wmove(editor_text_subwindow, y, x);
				txt[y * 97 + x] = key;
				wprintw(editor_text_subwindow, &key);
				x++;
			}
			break;
		}
	}
}