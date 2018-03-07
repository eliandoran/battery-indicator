#include <libintl.h>
#include <ncurses.h>
#include <string.h>
#include "../includes/DataTypes.h"
#include "../includes/ColorCodes.h"
#include "../includes/ncurses_utils.h"
#include "../includes/drawing.h"

const int info_padding = 1;

void draw_info(BATTERY* batt)
{
	batt->infoRect = (RECT){
		batt->battRect.top + 1,
		batt->battRect.left + batt->battRect.width + info_padding,
		40, 15
	};

	batt->infoWin = new_window(batt->infoRect, false);
}

void redraw_info(BATTERY* batt)
{
	bool newline = false;
	int len, color;
	
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
	
	wrefresh(batt->infoWin);
}

void print_info_section(BATTERY* batt, unsigned colmax, const char* section)
{
	wattron(batt->infoWin, A_BOLD);
	wprintw(batt->infoWin, "%-*s", colmax, section);
	wprintw(batt->infoWin, ": ");
	wattroff(batt->infoWin, A_BOLD);
}