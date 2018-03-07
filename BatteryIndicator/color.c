#include <ncurses.h>
#include "includes/types.h"

#define PAIR_BEST  7
#define PAIR_NEUTRAL  8
#define PAIR_FAIR  6
#define PAIR_WORST 5

void init_colors()
{
	init_pair(5, COLOR_RED,    -1);
	init_pair(6, COLOR_YELLOW, -1);
	init_pair(7, COLOR_GREEN,  -1);
	init_pair(8, COLOR_BLUE,   -1);
}

short get_state_color(unsigned short state)
{
	switch (state) {
		case STATE_CHARGING:
			return PAIR_NEUTRAL;
		case STATE_DISCHARGING:
			return PAIR_FAIR;
		case STATE_FULL:
			return PAIR_BEST;
		default:
			return PAIR_WORST;
	}
}

short get_wear_color(unsigned short wearPercentage)
{
	if (wearPercentage <= 10)
		return PAIR_BEST;
	else if (wearPercentage <= 25)
		return PAIR_FAIR;
	else
		return PAIR_WORST;
}

short get_level_color(unsigned short curLevel, unsigned short totalLevels)
{
	unsigned short percentage = ((float)curLevel / totalLevels) * 100;

	if (percentage <= 20)
		return PAIR_WORST;
	else if (percentage <= 70)
		return PAIR_FAIR;
	else
		return PAIR_BEST;
}
