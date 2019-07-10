#pragma once

#include <Arduino.h>

typedef struct {
  byte hour;
  byte minute;
  byte second;
  int millisecond;
} TimeResult;

void register_date_callback(void (*date_fetch_callback)(byte *, byte *, byte *));
void pps_interrupt();
void sensor_interrupt();
bool sensor_has_triggered();
bool currentTime(TimeResult *output);
unsigned long sensor_interrupt_micros();
void clear_sensor_interrupt_micros();
