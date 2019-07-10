#include "uni_keypad.h"
#include "uni_gps.h"
#include "uni_display.h"
#include "uni_printer.h"
#include "uni_sd.h"
#include "modes.h"

extern UniKeypad keypad;
extern UniGps gps;
extern UniDisplay display;
extern UniPrinter printer;
extern UniSd sd;

//### Mode 2 - GPS/Printer/SD Test
//
//- If you press A, it will show the GPS time, and beep positively.
//- If you press B, it will show print a test line on the printer.
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
      if (keynum == 18) {
        // B
        char test_string[] = "PRINTER TEST STRING";
        printer.print(test_string);
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
        int hour, minute, second, millisecond;
        bool res = gps.current_time(micros(), &hour, &minute, &second, &millisecond);
        Serial.print("Res: ");
        Serial.println(res);
        char data[20];
        sprintf(data, "%02d:%02d:%02d:%03d", hour, minute, second, millisecond);
        Serial.println(data);
      }
    }
  }   
  last_key2 = key;
}
