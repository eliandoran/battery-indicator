#include <ncurses.h>
#include "includes/DataTypes.h"

WINDOW* new_window(RECT rect, bool draw_border)
{
	WINDOW* win = newwin(rect.height, rect.width, rect.top, rect.left);

	if (draw_border) {
		box(win, 0, 0);
		wrefresh(win);
	}

	return win;
}