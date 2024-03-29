#include <ncurses.h>
#include "../includes/types.h"
#include "../includes/color.h"
#include "../includes/utils.h"

const int connector_w = 3;
const int scale_w = 3, scale_padding = 1;

int levels, level_percentage;

void init_connector(BATTERY* batt) {
	batt->connectorRect = (RECT){
		batt->battRect.top - 1,
		batt->battRect.left + ((batt->battRect.width - connector_w) / 2),
		connector_w, 2
	};

	batt->connectorWin = new_window(batt->connectorRect, true);
}

void draw_connector(BATTERY* batt)
{
	wborder(batt->connectorWin, 0, 0, 0, 0, 0, 0, ACS_BTEE, ACS_BTEE);
	wrefresh(batt->connectorWin);
}

void init_scale(BATTERY* batt) {		
	level_percentage = 100 / (batt->battRect.height - 2);
	levels = 100 / level_percentage;	
	
	batt->scaleRect = (RECT){
		batt->battRect.top + 1,
		batt->battRect.left - scale_w - scale_padding,
		scale_w, levels
	};

	batt->scaleWin = new_window(batt->scaleRect, false);
}

void draw_scale(BATTERY* batt)
{	
	int i;
	for (i=0; i<levels; i++) {
		mvwprintw(batt->scaleWin, i, 0, "%3d%%", 100 - (level_percentage * i));
	}

	wrefresh(batt->scaleWin);
}

void init_level(BATTERY* batt) {
	batt->batteryWin = new_window(batt->battRect, true);
}

void draw_level(BATTERY* batt)
{
	unsigned short cur_level = (int)(batt->info->capacity / level_percentage),
				   lvl = levels - cur_level,
				   i, j;
	int color;	

	for (i=0; i < levels; i++) {
		wmove(batt->batteryWin, i+1, 1);
		color = COLOR_PAIR(get_level_color(cur_level, levels));
		wattron(batt->batteryWin, color);
		for (j=0; j < batt->battRect.width - 2; j++)
		{
			waddch(batt->batteryWin, (i >= lvl ? ACS_BOARD : ' '));
		}
		wattroff(batt->batteryWin, color);
	}

	wrefresh(batt->batteryWin);
}
