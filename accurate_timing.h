#pragma once

#include <Arduino.h>

void register_date_callback(void (*date_fetch_callback)(byte *, byte *, byte *));
void pps_interrupt();
void sensor_interrupt();
bool sensor_has_triggered();
bool currentTime(char *output);
unsigned long sensor_interrupt_micros();
void clear_sensor_interrupt_micros();
