#pragma once

#include "accurate_timing.h"
void store_racer_number();
void clear_racer_number();
int racer_number();
bool three_digits_racer_number();

// ****************
void build_race_filename(char *filename, const int max_length);
void print_racer_data_to_sd(int racer_number, TimeResult data);
void clear_previous_entry();
void log(char *message);

#include "uni_config.h"
Config *getConfig();

// ****************
