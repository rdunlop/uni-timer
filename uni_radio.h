#ifndef UNI_RADIO_H
#define UNI_RADIO_H

#include <SPI.h>
#include <RH_RF95.h>

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

class UniRadio
{
  public:
    UniRadio(int cs, int interrupt) : _interrupt(interrupt), _rf95(cs, interrupt) {};
    void setup();
    void loop();
    bool status();
  private:
    bool _status;
    uint8_t _interrupt;
    RH_RF95 _rf95;
};

#endif
