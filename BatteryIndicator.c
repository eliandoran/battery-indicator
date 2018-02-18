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
#include "include/DataTypes.h"
#include "include/DataGrabber.h"
#include "include/ColorCodes.h"

const int connector_w = 3;
const int scale_w = 3, scale_padding = 1;
const int info_padding = 1;
int levels, level_percentage;
short blinking_percentage = 15;

unsigned refresh_delay = 900,
		 blink_delay   = 500,
		 wait_delay    = 150;

BATTERY_LIST batteries;

WINDOW* new_window(RECT rect, bool draw_border)
{
	WINDOW* win = newwin(rect.height, rect.width, rect.top, rect.left);

	if (draw_border) {
		box(win, 0, 0);
		wrefresh(win);
	}

	return win;
}

void draw_connector(BATTERY* batt)
{
	batt->connectorRect = (RECT){
		batt->battRect.top - 1,
		batt->battRect.left + ((batt->battRect.width - connector_w) / 2),
		connector_w, 2
	};

	batt->connectorWin = new_window(batt->connectorRect, true);

	wborder(batt->connectorWin, 0, 0, 0, 0, 0, 0, ACS_BTEE, ACS_BTEE);
	wrefresh(batt->connectorWin);
}

void draw_scale(BATTERY* batt)
{
	level_percentage = 100 / (batt->battRect.height - 2);
	levels = 100 / level_percentage;

	batt->scaleRect = (RECT){
		batt->battRect.top + 1,
		batt->battRect.left - scale_w - scale_padding,
		scale_w, levels
	};

	batt->scaleWin = new_window(batt->scaleRect, false);

	int i;
	for (i=0; i<levels; i++) {
		mvwprintw(batt->scaleWin, i, 0, "%3d%%", 100 - (level_percentage * i));
	}

	wrefresh(batt->scaleWin);
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
			waddch(batt->batteryWin, (i >= lvl ? 'x' : ' '));
		}
		wattroff(batt->batteryWin, color);
	}

	wrefresh(batt->batteryWin);
}

void draw_info(BATTERY* batt)
{
	batt->infoRect = (RECT){
		batt->battRect.top + 1,
		batt->battRect.left + batt->battRect.width + info_padding,
		35, 15
	};

	batt->infoWin = new_window(batt->infoRect, false);
}

void init()
{
	setlocale(LC_ALL, "");
	initscr();
	start_color(); init_colors();
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
	rows[6] = gettext("Current Charge");
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

	// Current Charge
	print_info_section(batt, colmax, rows[6]);
	wprintw(batt->infoWin, "%d mA / %d mA\n", batt->info->charge_now / 1000, batt->info->charge_full / 1000);

	// Technology
	print_info_section(batt, colmax, rows[7]);
	wprintw(batt->infoWin, "%s\n", batt->info->technology);

	batt->is_blinking = (batt->info->capacity <= blinking_percentage);
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
		batt->blinked = batt->is_blinking = false;
		batt->battRect = (RECT){ 2 + batt_h * i + batt_padding * i, 7, 11, batt_h };
		batt->batteryWin = new_window(batt->battRect, true);
		draw_scale(batt);
		draw_info(batt);
		draw_connector(batt);
		update_info(batt);
	}
}

void update_blink_status(BATTERY* batt)
{
	if (batt->is_blinking) {
		batt->blinked = !batt->blinked;

		if (batt->blinked) {
			werase(batt->batteryWin);
			werase(batt->connectorWin);
			wrefresh(batt->batteryWin);
			wrefresh(batt->connectorWin);
		} else {
			box(batt->batteryWin, 0, 0);
			wrefresh(batt->batteryWin);
			draw_connector(batt);
			wrefresh(batt->connectorWin);
		}

		wrefresh(batt->batteryWin);
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
