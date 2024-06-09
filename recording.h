#pragma once

#include "accurate_timing.h"
void store_racer_number();
void clear_racer_number();
int racer_number();
bool maximum_digits_racer_number();

// ****************
void build_race_filename(char *filename, const int max_length);
bool print_racer_data_to_sd(int racer_number, TimeResult data, bool fault = false);
void print_data_to_log(TimeResult data, bool fault = false);
void clear_previous_entry();
void log(const char *message);

#include "uni_config.h"
Config *getConfig();
#define RECENT_RESULT_COUNT 9
extern TimeResult recentResult[RECENT_RESULT_COUNT];
extern int recentRacer[RECENT_RESULT_COUNT];
// ****************
