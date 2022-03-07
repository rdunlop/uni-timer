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

typedef struct {
  bool start;
  uint8_t difficulty; // 0-B, 1-A, 2-E
  bool up;
  uint8_t number;    
//  bool start = true;
//  uint8_t difficulty = 0; // 0-B, 1-A, 2-E
//  bool up = true;
//  uint8_t number = 1;    
} Config;

Config *getConfig();

// ****************
