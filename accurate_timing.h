#pragma once

#include <Arduino.h>
#include "uni_gps.h"

void pps_interrupt();
void sensor_interrupt();
bool sensor_has_triggered();
bool lastSensorTime(TimeResult *output);
bool currentTime(TimeResult *output);
unsigned long sensor_interrupt_millis();
void clear_sensor_interrupt_millis();
