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
#include "drawing/drawing.h"
#include "ncurses_utils.h"

const int info_padding = 1;
short blinking_percentage = 15;

unsigned refresh_delay = 900,
		 blink_delay   = 1000,
		 wait_delay    = 100;

BATTERY_LIST batteries;

void draw_info(BATTERY* batt)
{
	batt->infoRect = (RECT){
		batt->battRect.top + 1,
		batt->battRect.left + batt->battRect.width + info_padding,
		40, 15
	};

	batt->infoWin = new_window(batt->infoRect, false);
}

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

void print_info_section(BATTERY* batt, unsigned colmax, const char* section)
{
	wattron(batt->infoWin, A_BOLD);
	wprintw(batt->infoWin, "%-*s", colmax, section);
	wprintw(batt->infoWin, ": ");
	wattroff(batt->infoWin, A_BOLD);
}

void update_info(BATTERY* batt)
{
	bool newline = false;
	int len, color;

	get_battery_info(batt, false);
	werase(batt->infoWin);

	// Manufacturer
	if (strlen(batt->info->manufacturer)) {
		wprintw(batt->infoWin, "%s ", batt->info->manufacturer);
		newline = true;
	}

	// Model Name
	if (strlen(batt->info->model_name)) {
		wprintw(batt->infoWin, batt->info->model_name);
		newline = true;
	}

	if (newline) {
		wprintw(batt->infoWin, "\n\n");
		newline = false;
	}

	// State
	#define NUM_ROWS 8
	char* rows[NUM_ROWS];
	rows[0] = gettext("State");
	rows[1] = gettext("Percentage");
	rows[2] = gettext("Battery Wear");
	rows[3] = gettext("Voltage");
	rows[4] = gettext("Cycle Count");
	rows[5] = gettext("Serial Number");
	rows[6] = gettext("Current Energy");
	rows[7] = gettext("Technology");

	int colmax=strlen(rows[0]), i;
	for (i=1; i<NUM_ROWS; i++) {
		len = strlen(rows[i]);
		if (len > colmax) colmax = len;
	}
	#undef NUM_ROWS

	print_info_section(batt, colmax, rows[0]);
 	color = COLOR_PAIR(get_state_color(batt->info->state));
	wattron(batt->infoWin, color);
	switch (batt->info->state) {
		case STATE_FULL:
			wprintw(batt->infoWin, gettext("Full\n"));
			break;

		case STATE_CHARGING:
			wprintw(batt->infoWin, gettext("Charging\n"));
			break;

		case STATE_DISCHARGING:
			wprintw(batt->infoWin, gettext("Discharging\n"));
			break;

		default:
			wprintw(batt->infoWin, gettext("Unknown\n"));
			break;
	}
	wattroff(batt->infoWin, color);

	// Percentage
	print_info_section(batt, colmax, rows[1]);
	color = COLOR_PAIR(get_level_color((int)batt->info->capacity, 100));
	wattron(batt->infoWin, color);
	wprintw(batt->infoWin, "%.2f%%\n", batt->info->capacity);
	wattroff(batt->infoWin, color);

	// Battery Wear
	print_info_section(batt, colmax, rows[2]);
	color = COLOR_PAIR(get_wear_color((int)batt->info->battery_wear));
	wattron(batt->infoWin, color);
	wprintw(batt->infoWin, "%.2f%%\n", batt->info->battery_wear);
	wattroff(batt->infoWin, color);

	// Voltage
	print_info_section(batt, colmax, rows[3]);
	wprintw(batt->infoWin, "%.2f V\n", (float)batt->info->voltage_now / 1000000);

	// Cycle Count
	if (batt->info->cycle_count) {
		print_info_section(batt, colmax, rows[4]);
		wprintw(batt->infoWin, "%d\n", batt->info->cycle_count);
	}

	// Serial Number
	len = strlen(batt->info->serial_no);
	if (len > 1 || (len == 1 && batt->info->serial_no[0] != ' ')) {
		print_info_section(batt, colmax, rows[5]);
		wprintw(batt->infoWin, "%s\n", batt->info->serial_no);
	}

	// Current Energy
	print_info_section(batt, colmax, rows[6]);
	wprintw(batt->infoWin, "%d mWh / %d mWh\n", batt->info->energy_now / 1000, batt->info->energy_full / 1000);

	// Technology
	print_info_section(batt, colmax, rows[7]);
	wprintw(batt->infoWin, "%s\n", batt->info->technology);

	batt->is_blinking = (batt->info->capacity <= blinking_percentage);
	//batt->is_blinking = true;
	
	if (!batt->is_blinking)	
		draw_level(batt);
		
	wrefresh(batt->infoWin);
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
