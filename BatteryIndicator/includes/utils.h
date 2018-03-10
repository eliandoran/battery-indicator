#include <ncurses.h>
#include "types.h"

// utils/ncurses_utils.c
WINDOW* new_window(RECT rect, bool draw_border);
void enable_colors();

// utils/time_utils.c
void sleep_ms(int delay);

// utils/file_utils.c
void file_to_str(const char* filePath, char* outputStr, int outputLen);
int file_to_int(const char* filePath);