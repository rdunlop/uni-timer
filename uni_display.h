#ifndef UNI_DISPLAY_H
#define UNI_DISPLAY_H

// #define SEVEN_SEGMENT_DISPLAY true
#define LCD_DISPLAY true

#ifdef SEVEN_SEGMENT_DISPLAY
#include "Adafruit_LEDBackpack.h"
#endif
#ifdef LCD_DISPLAY
#include "Adafruit_LiquidCrystal.h"
#endif
#include "uni_gps.h" // for TimeResult
#include "uni_config.h" // for UniConfig

class UniDisplay
{
  public:
    UniDisplay(int i2c_addr, int lcd_i2c_addr) :
        _i2c_addr(i2c_addr),
    #ifdef SEVEN_SEGMENT_DISPLAY
        _display(),
    #endif
    #ifdef LCD_DISPLAY
       _lcd(Adafruit_LiquidCrystal(lcd_i2c_addr)),
    #endif
        _wait_state(0) {}
    void setup();

    // API
    void sdGood(bool delay_for_segment = false);
    void sdBad(bool delay_for_segment = false);
    void radioGood(bool delay_for_segment = false);
    void radioBad(bool delay_for_segment = false);
    void gpsGood(bool delay_for_segment = false);
    void gpsBad(bool delay_for_segment = false);
    void allGood();
    void notAllGood();
    void displayTest();
    void sens();
    void waitingForSensor(const int racer_number, const bool countdown);
    void doneWaitingForSensor();
    void show(char);
    void print(const char *message = NULL, const char *message2 = NULL);
    void showTimeResult(TimeResult *time_result);
    void showNumber(int);
    void displayConfig(UniConfig *config);
    void displayRadioConfig(UniConfig *config);
    void showRacerDigits(int);
    void startLineCountdown(bool, uint8_t);
    void triggerIntervalDelay(uint16_t);
    void showEntriesRemaining(int);
    void waitingForGps();
    void doneWaitingForGps();
    void waitingPattern();
    void configNotFound();
    void configLoaded();
    // Internal
    void all();
    void good();
    void bad();
    void sd();
    void gps();
    void radio();
    void setBlink(bool blink);
    void clear();
    void showWaiting(bool, int segment);
  private:
    int _i2c_addr;
    #ifdef SEVEN_SEGMENT_DISPLAY
    Adafruit_7segment _display;
    #endif
    #ifdef LCD_DISPLAY
    Adafruit_LiquidCrystal _lcd;
    #endif
    int _wait_state;
    char _buffer[2][17];
};
#endif
