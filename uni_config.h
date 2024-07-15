#ifndef UNI_CONFIG_H
#define UNI_CONFIG_H

#include <Arduino.h>

#define MAX_RADIO_ID 10
// NOTE: Adjustments to this structure MUST ALSO
// be reflected in the readConfig and writeConfig methods
// or else the config won't persist/parse properly
#define FILENAME_MAX_LENGTH 100
typedef struct {
  // filename parts
  bool start; //[T, F]
  uint8_t difficulty; // [B, A, E]
  bool up; // [T, F]
  uint8_t race_number; // [0..9]
  // filename whole
  char filename[FILENAME_MAX_LENGTH];

  // Racer bib number
  uint8_t bib_number_length;

  // Start line modes
  bool start_line_countdown;
  // mode 0 -> multi-beep: 5 normal beeps, then a single start beep
  // mode 1 -> single beep, then starts clock 3 if they don't cross within 3 seconds
  uint8_t start_line_countdown_mode;

  // Finish line spacing
  uint16_t finish_line_spacing;

  // Resume the race mode stored
  int mode;

  bool radio_enabled;
  uint8_t radio_id;
  uint8_t radio_target_id;
} Config;

class UniConfig
{

  public:
    UniConfig();
    void setup();
    bool loadedFromDefault();

    // Radio configuration
    bool radioEnabled();
    uint8_t radioID();
    uint8_t radioTargetID();
    void toggleRadioEnabled();
    void incrementRadioID();
    void incrementRadioTargetID();

    void toggle_start();
    void increase_difficulty();
    void toggle_up();
    void increment_race_number();
    bool get_start();
    int get_difficulty();
    bool get_up();
    int get_race_number();

    void toggle_bib_number_length();
    int get_bib_number_length();

    // start_line_countdown
    void toggle_start_line_countdown();
    bool get_start_line_countdown();
    int get_start_line_countdown_mode();
    void increment_start_line_countdown_mode();

    // finish_line_spacing
    void reset_finish_line_spacing();
    void increment_finish_line_spacing(int ms);
    int get_finish_line_spacing();

    char *filename();
    int mode();
    void setMode(int mode);
    bool writeConfig();
  private:
    bool readConfig();
    bool _loadedFromDefault;
    bool prefix(const char *str, const char *prefix);
    char *value(const char *str, const char *prefix);
    Config _config;
};
#endif
