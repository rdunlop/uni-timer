#pragma once

#include "accurate_timing.h"
void store_racer_number(int racer_number);
void clear_racer_number();
int racer_number();
bool three_digits_racer_number();

// ****************
void build_race_filename(char *filename, const int max_length);
void publish_time_recorded(int racer_number, char *data);
void print_racer_data_to_printer(int racer_number, TimeResult data);
void print_racer_data_to_sd(int racer_number, TimeResult data);
void clear_previous_entry();
char *filename();
void set_filename(const char *filename);

typedef struct {
  char filename[40];  
} Config;

Config *getConfig();

// ****************
