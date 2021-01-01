#include "uni_sd.h"
#include "recording.h"
#include "event_queue.h"
#include "uni_buzzer.h"

#ifdef ENABLE_SD
extern UniSd sd;
#endif

#ifdef ENABLE_BUZZER
extern UniBuzzer buzzer;
#endif

/* ********** Current-RACER NUMBER globals ****************************** */
int _racer_number = 0;

// Add a new digit to the current racer number
void store_racer_number(int racer_number) {
  _racer_number = racer_number;
  Serial.print("Racer #: ");
}

// Methods
void clear_racer_number() {
  _racer_number = 0;
}

int racer_number() {
  return _racer_number;
}
/* *************** PUBLISH Listener ************************* */
void push_racer_number(int racer_number, char *data) {
  char data_string[EVT_MAX_STR_LEN];
  snprintf(data_string, EVT_MAX_STR_LEN, "%d,%s", racer_number, data);
  push_event(EVT_TIME_RECORD, data_string);
}

void publish_time_recorded(uint8_t event_type, char *event_data) {
  switch(event_type) {
    case EVT_TIME_RECORD:
      Serial.println("Recording Racer");
      Serial.println(event_data);
      bool success = false;
#ifdef ENABLE_SD
      success = sd.writeFile("HELLO.txt", event_data);
#else
      success = true;
#endif

#ifdef ENABLE_BUZZER
      if (!success) {
        buzzer.error();
      }
#endif
      push_event(EVT_TIME_STORED, event_data);
  }
}
