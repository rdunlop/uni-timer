#ifndef UNI_DISPLAY_H
#define UNI_DISPLAY_H
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_LiquidCrystal.h"
#include "uni_gps.h" // for TimeResult
#include "uni_config.h" // for UniConfig

class UniDisplay
{
  public:
    UniDisplay(int i2c_addr, int lcd_i2c_addr) :
        _i2c_addr(i2c_addr), _display(), _wait_state(0), _lcd(Adafruit_LiquidCrystal(lcd_i2c_addr)) {}
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
    void waitingForSensor();
    void doneWaitingForSensor();
    void show(char);
    void print(const char *message, const char *message2);
    void showTimeResult(TimeResult *time_result);
    void showNumber(int);
    void displayConfig(UniConfig *config);
    void showRacerDigits(int);
    void startLineCountdown(bool);
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
    Adafruit_7segment _display;
    int _wait_state;
    Adafruit_LiquidCrystal _lcd;
};
#endif
