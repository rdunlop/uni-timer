#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_sd.h"
#include "modes.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniSd sd;

//### Mode 2 - GPS/SD Test
//
//- If you press A, it will show the GPS time, and beep positively.
//- If you press C, it will test writing/reading from the SD card, and display either 6ood or bAd
unsigned long gps_millis = 0;
char last_key2 = NO_KEY;
void mode2_loop() {
  if (gps_millis == 0 || (millis() - gps_millis > 100)) {
    gps_millis = millis();
    gps.printGPSDate();
  }
  char key = keypad.readChar();
  if (key != NO_KEY) {
    if (key != last_key2) {
      // New Keypress
      int keynum = keypad.intFromChar(key);
      if (keynum == 17) {
        // A
        int hour, minute, second;
        gps.printPeriodically();
        gps.getHourMinuteSecond(&hour, &minute, &second);
        display.showNumber((minute * 100) + second, DEC);  
      }
      if (keynum == 19) {
        // C
        if (sd.status()) {
          display.good();
        } else {
          display.bad();
        }
      }
      if (keynum == 20) {
        byte hour, minute, second;
        int millisecond = 0;
    
        bool res = gps.current_time(&hour, &minute, &second);
        Serial.print("Res: ");
        Serial.println(res);
        char data[20];
        snprintf(data, 20, "%02d:%02d:%02d:%03d", hour, minute, second, millisecond);
        Serial.println(data);
      }
    }
  }   
  last_key2 = key;
}