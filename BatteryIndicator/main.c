/*
 * Battery Indicator
 * © 2015 Elian Doran <elinnum@gmail.com>
 *
 * This is a small Linux application that displays a textual representation of a battery, as well as some
 * information (e.g. voltage, whether it is charging or not, etc.).
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <ncurses.h>
#include <time.h>
#include "includes/DataTypes.h"
#include "includes/DataGrabber.h"
#include "includes/ColorCodes.h"
#include "includes/ncurses_utils.h"
#include "includes/drawing.h"

unsigned refresh_delay = 900,
		 blink_delay   = 1000,
		 wait_delay    = 100;

short blinking_percentage = 15;

BATTERY_LIST batteries;

void enable_colors() {
	if (!has_colors())
		return;
		
	use_default_colors();	
	start_color();	
	init_colors();
}

void init()
{
	setlocale(LC_ALL, "");
	initscr();
	enable_colors();	
	cbreak();
	noecho();
	keypad(stdscr, true);
	nodelay(stdscr, true);
	curs_set(0);
	refresh();
}

void release()
{
	unsigned i;
	BATTERY* batt;
	for (i=0, batt=&batteries.items[i]; i<batteries.count; i++) {
		delwin(batt->infoWin);
		delwin(batt->scaleWin);
		delwin(batt->connectorWin);
		delwin(batt->batteryWin);
	}
	endwin();
}

void sleep_ms(int delay)
{
	const struct timespec sleep_ts = {
		delay / 1000,
		(delay % 1000) * 1000000
	};

	nanosleep(&sleep_ts, NULL);
}

void update_blink_status(BATTERY* batt)
{
	if (batt->is_blinking) {
		batt->blinked = !batt->blinked;

		if (batt->blinked) {
			werase(batt->batteryWin); 
			werase(batt->connectorWin);
			werase(batt->scaleWin);
		} else {
			box(batt->batteryWin, 0, 0);
			draw_level(batt);
			draw_connector(batt);
			draw_scale(batt);			
		}
				
		wrefresh(batt->batteryWin);
		wrefresh(batt->connectorWin);
		wrefresh(batt->scaleWin);
	}
}

void load_batteries()
{
	unsigned i;
	int batt_h = 12,
		batt_padding = 2;

	batteries.count = 1;
	batteries.items = (BATTERY*)malloc(sizeof(BATTERY) * batteries.count);

	BATTERY* batt;
	for (i=0, batt=&batteries.items[i]; i<batteries.count; i++) {
		batt->id = 0;
		batt->is_blinking = false;
		batt->blinked = true;
		batt->battRect = (RECT){ 2 + batt_h * i + batt_padding * i, 7, 11, batt_h };
		batt->batteryWin = new_window(batt->battRect, true);
		
		draw_scale(batt);
		draw_info(batt);
		draw_connector(batt);
		update_info(batt);				
		update_blink_status(batt);
	}
}

void update_info(BATTERY* batt)
{	
	get_battery_info(batt, false);	
	
	redraw_info(batt);

	batt->is_blinking = (batt->info->capacity <= blinking_percentage);
	//batt->is_blinking = true;
	
	if (!batt->is_blinking)	
		draw_level(batt);			
}

int main()
{
	unsigned refresh_time = 0,
			 blink_time = 0,
			 i;
	BATTERY* batt;

	init();
	load_batteries();
	
	char ch;
	while (ch=getch(), ch != 27 && ch != 'q') {				
		if (blink_time >= blink_delay) {
			for (i=0, batt=&batteries.items[i]; i<batteries.count; i++)
				update_blink_status(batt);

			blink_time = 0;
		}

		if (refresh_time >= refresh_delay) {
			for (i=0, batt=&batteries.items[i]; i<batteries.count; i++)
				update_info(batt);

			refresh_time = 0;
		}

		sleep_ms(wait_delay);
		blink_time += wait_delay;
		refresh_time += wait_delay;
	}

	release();
	return EXIT_SUCCESS;
}
