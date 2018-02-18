#ifndef COLORCODES_H_INCLUDED
#define COLORCODES_H_INCLUDED

void init_colors();
short get_state_color(unsigned short state);
short get_wear_color(unsigned short wearPercentage);
short get_level_color(unsigned short curLevel, unsigned short totalLevels);

#endif // COLORCODES_H_INCLUDED
