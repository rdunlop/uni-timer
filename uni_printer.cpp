// - PRINTER
#include "uni_printer.h"

UniPrinter::UniPrinter(int rx_pin, int tx_pin)
{
   _rx_pin = rx_pin;
   _tx_pin = tx_pin;
}

void UniPrinter::setup() {
  Serial2.begin(19200, SERIAL_8N1, _rx_pin, _tx_pin);
  printer = new Adafruit_Thermal(&Serial2);     // Pass addr to printer constructor

  printer->begin();
//  printer->doubleHeightOn();
  printer->inverseOff();
  Serial.println("Printer Done init");
}

void UniPrinter::loop() { }

boolean UniPrinter::hasPaper() {
  return printer->hasPaper();
}

void UniPrinter::hello() { 
  printer->println("Hello Robin");
  printer->feed(2);
  printer->sleep();
}

void UniPrinter::print(char *str) { 
  printer->wake();
  printer->println(str);
  printer->sleep();
}

void UniPrinter::feed() {
  printer->feed(2);
}
