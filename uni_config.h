#ifndef UNI_CONFIG_H
#define UNI_CONFIG_H

#include <Arduino.h>

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

  // Finish line spacing
  uint16_t finish_line_spacing;

  // Resume the race mode stored
  int mode;
} Config;

class UniConfig
{

  public:
    UniConfig();
    void setup();
    bool loadedFromDefault();

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

    // finish_line_spacing
    void reset_finish_line_spacing();
    void increment_finish_line_spacing(int ms);
    int get_finish_line_spacing();

    char *filename();
    int mode();
    void setMode(int mode);
  private:
    bool readConfig();
    bool _loadedFromDefault;
    bool writeConfig();
    bool prefix(const char *str, const char *prefix);
    char *value(const char *str, const char *prefix);
    Config _config;
};
#endif
