#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

char** wnd_print(DIR* dir, WINDOW* wnd, char** names, int** types, int* string) {

	names = (char**)malloc(256);
	types = (int**)malloc(256);

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
		strcpy(names[i], data->d_name);
		i++;
	}
	names = (char**)realloc(names, sizeof(char*) * (i + 1));

	*string = i;

    return names;
}

char** change(DIR** open, WINDOW* ptrtxt, WINDOW* ptrpath, char** ptrnames, int* types, char** buftmp, int ptrstr, int *string) {

	rewinddir(*open);
	int size = 0;
	//char* buf = buftmp;

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

	wmove(ptrpath, 1, 1);
	wprintw(ptrpath, *buftmp);
	ptrnames = wnd_print(*open, ptrtxt, ptrnames, &types, string);
	box(ptrpath, '|', '-');
	wrefresh(ptrpath);
	wrefresh(ptrtxt);

	return ptrnames;
}

int main(int args, char** argv) {

	initscr();
	curs_set(FALSE);
	noecho();
	cbreak();

	WINDOW* left = newwin(40, 70, 0, 0);
	WINDOW* right = newwin(40, 70, 0, 71);
	WINDOW* ptrpath = left;

	WINDOW* subwndl = derwin(left, 35, 60, 3, 5);
	WINDOW* subwndr = derwin(right, 35, 60, 3, 5);
	WINDOW* ptrtxt = subwndl;

	WINDOW* ptrleft = derwin(left, 35, 2, 3, 1);
	WINDOW* ptrright = derwin(right, 35, 2, 3, 1);
	WINDOW* ptr = ptrleft;

	keypad(stdscr, TRUE);
	refresh();

    char* bufl = malloc(128);
	char* bufr = malloc(128);
	char* buftmp = bufl;;
	char** namesl;
	char** namesr;

	int stringl = 0;
    int stringr = 0;
	int ptrstr = 0;
	int key;
	int* typel;
	int* typer;

	getcwd(bufl, 128);
	getcwd(bufr, 128);

	DIR* open = opendir(bufl);
	
	box(left, '|', '-');
	box(right, '|', '-');

	wmove(left, 1, 1);
	wprintw(left, bufl);

	wmove(right, 1, 1);
	wprintw(right, bufr);

	namesr = wnd_print(open, subwndr, namesr, &typer, &stringr);
	namesl = wnd_print(open, subwndl, namesl, &typel, &stringl);
	
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
				}
				else {
					ptr = ptrleft;
				}
                break;
			case 10: //ENTER
					if (ptr == ptrleft) {
						namesl = change(&open, subwndl, left, namesl, typel, &bufl, ptrstr, &stringl);
					}
					else{
						namesr = change(&open, subwndr, right, namesr, typer,  &bufr, ptrstr, &stringr);
					}
					if (namesl == namesr)
					{
						delwin(left);
						delwin(right);
						refresh();
						endwin();
						system("clear");
						
						printf("error: namesl=%p namesr=%p\n", namesl, namesr);

						exit(EXIT_SUCCESS);
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