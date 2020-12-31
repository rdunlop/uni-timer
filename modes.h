#pragma once

void mode1_event_handler(uint8_t event_type, char *event_data);

void mode3_setup();
void mode3_event_handler(uint8_t event_type, char *event_data);
void mode3_teardown();

void mode4_setup();
void mode4_event_handler(uint8_t event_type, char *event_data);
void mode4_teardown();

void mode5_setup();
void mode5_event_handler(uint8_t event_type, char *event_data);
void mode5_teardown();

void mode6_setup();
void mode6_event_handler(uint8_t event_type, char *event_data);
void mode6_teardown();
