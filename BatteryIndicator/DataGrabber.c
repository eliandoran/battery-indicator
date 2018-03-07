#ifndef _DATA_GRABBER
#define _DATA_GRABBER 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "includes/DataTypes.h"

static void file_to_str(const char* filePath, char* outputStr, int outputLen)
{
	FILE* file = fopen(filePath, "r");
	int len;
	fgets(outputStr, outputLen, file);
	len = strlen(outputStr);
	if (outputStr[len - 1] == '\n') outputStr[len - 1] = '\0';
	fclose(file);
}

static int file_to_int(const char* filePath)
{
	FILE* file = fopen(filePath, "r");
	int result;
	fscanf(file, "%d", &result);
	fclose(file);
	return result;
}

void get_battery_info(BATTERY* batt, bool capacity_by_design)
{
	const char path_fmt[] = "/sys/class/power_supply/BAT%d/%s";
	char path[100];
	BATTERY_INFO* result = (BATTERY_INFO*)malloc(sizeof(BATTERY_INFO));

	// Manufacturer
	sprintf(path, path_fmt, batt->id, "manufacturer");
	file_to_str(path, result->manufacturer, MANUFACTURER_LENGTH);

	// Model Name
	sprintf(path, path_fmt, batt->id, "model_name");
	file_to_str(path, result->model_name, MODEL_LENGTH);

	// Energy Now
	sprintf(path, path_fmt, batt->id, "energy_now");
	result->energy_now = file_to_int(path);

	// Energy Full
	sprintf(path, path_fmt, batt->id, "energy_full");
	result->energy_full = file_to_int(path);

	// Energy Full Design
	sprintf(path, path_fmt, batt->id, "energy_full_design");
	result->energy_full_design = file_to_int(path);

	// Battery Wear
	result->battery_wear = ((float)(result->energy_full_design - result->energy_full) / result->energy_full_design) * 100;

	// Voltage Now
	sprintf(path, path_fmt, batt->id, "voltage_now");
	result->voltage_now = file_to_int(path);

	// Voltage Min Design
	sprintf(path, path_fmt, batt->id, "voltage_min_design");
	result->voltage_min_design = file_to_int(path);

	// Cycle Count
	sprintf(path, path_fmt, batt->id, "cycle_count");
	result->cycle_count = file_to_int(path);

	// Technology
	sprintf(path, path_fmt, batt->id, "technology");
	file_to_str(path, result->technology, TECHNOLOGY_LENGTH);

	// Serial Number
	sprintf(path, path_fmt, batt->id, "serial_number");
	file_to_str(path, result->serial_no, SERIAL_NO_LENGTH);

	// Capacity
	result->capacity = ((float)result->energy_now / (float)(capacity_by_design ? result->energy_full_design : result->energy_full)) * 100;

	// State
	char status[50];
	sprintf(path, path_fmt, batt->id, "status");
	file_to_str(path, status, 50);

	if (!strcmp(status, "Charging"))
		result->state = STATE_CHARGING;
	else if (!strcmp(status, "Discharging"))
		result->state = STATE_DISCHARGING;
	else if (!strcmp(status, "Full"))
		result->state = STATE_FULL;
	else
		result->state = STATE_UNKNOWN;

	if (batt->info != NULL) free(batt->info);
	batt->info = result;
}
#endif
