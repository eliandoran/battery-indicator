#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <ncurses.h>

#define MANUFACTURER_LENGTH 100
#define MODEL_LENGTH 100
#define TECHNOLOGY_LENGTH 100
#define SERIAL_NO_LENGTH 100

#define STATE_CHARGING 1
#define STATE_DISCHARGING 2
#define STATE_FULL 3
#define STATE_UNKNOWN 0

typedef struct {
    int top;
    int left;
    int width;
    int height;
} RECT;

typedef struct {
    char manufacturer[MANUFACTURER_LENGTH];
    char model_name[MODEL_LENGTH];
    int energy_now;
    int energy_full;
    int energy_full_design;
    float battery_wear;
    int voltage_now;
    int voltage_min_design;
    int cycle_count;
    char technology[TECHNOLOGY_LENGTH];
    char serial_no[SERIAL_NO_LENGTH];
    float capacity;
    int state;
} BATTERY_INFO;

typedef struct {
    int id;
    BATTERY_INFO* info;
    WINDOW *infoWin;
    WINDOW *scaleWin;
    WINDOW *connectorWin;
    WINDOW *batteryWin;
    RECT connectorRect;
    RECT battRect;
    RECT scaleRect;
    RECT infoRect;
    bool is_blinking;
    bool blinked;
} BATTERY;

typedef struct {
    BATTERY *items;
    int count;
} BATTERY_LIST;

#endif // TYPES_H_INCLUDED
