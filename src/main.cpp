#include <ncurses.h>

int main() {
    initscr();
    noecho();
    curs_set(0);


    mvprintw(0, 0, "Hello!");
    refresh();

    getch();

    endwin();
    return 0;
}
