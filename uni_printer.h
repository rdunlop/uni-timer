#ifndef UNI_PRINTER_H
#define UNI_PRINTER_H
#include <SoftwareSerial.h>
#include <Adafruit_Thermal.h>

class UniPrinter
{
  public:
    UniPrinter(int,int);
    void setup();
    void loop();
    void print(char *);
    void feed();
    void hello();
    boolean hasPaper();
  private:
    int _tx_pin;
    int _rx_pin;
    Adafruit_Thermal *printer;
    SoftwareSerial *printer_serial;
};

#endif
