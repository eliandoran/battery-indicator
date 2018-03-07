#include <ncurses.h>
#include "../includes/DataTypes.h"
#include "../includes/ColorCodes.h"

WINDOW* new_window(RECT rect, bool draw_border)
{
	WINDOW* win = newwin(rect.height, rect.width, rect.top, rect.left);

	if (draw_border) {
		box(win, 0, 0);
		wrefresh(win);
	}

	return win;
}

void enable_colors() {
	if (!has_colors())
		return;
		
	use_default_colors();	
	start_color();	
	init_colors();
}