#ifndef UNI_RADIO_H
#define UNI_RADIO_H

#include <SPI.h>
#include <RH_RF95.h>

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

class UniRadio
{
  public:
    UniRadio(int cs, int interrupt) : _enabled(false), _initialized(false), _interrupt(interrupt), _rf95(cs, interrupt) {};
    void setup(bool enabled, uint8_t radio_id, uint8_t radio_target_id);
    void loop();
    bool messageAvailable();
    bool receive(uint8_t *message, uint8_t *message_length);
    bool queueToSend(char *message);
    int queueSize();
    void displayQueue();
    bool status();
    bool senderTest();
    bool receiverTest(char *, int *);
  private:
    bool _enabled;
    bool _initialized;
    bool _status;
    uint8_t _interrupt;
    RH_RF95 _rf95;
};

#endif
