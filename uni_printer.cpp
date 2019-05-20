// - PRINTER
#include "uni_printer.h"

UniPrinter::UniPrinter(int rx_pin, int tx_pin)
{
   _rx_pin = rx_pin;
   _tx_pin = tx_pin;
}

void UniPrinter::setup() {
  printer_serial = new SoftwareSerial(_rx_pin, _tx_pin); // Declare SoftwareSerial obj first
  printer = new Adafruit_Thermal(printer_serial);     // Pass addr to printer constructor

  printer_serial->begin(19200); // this printer has a 19200 baud
  printer->begin();
  printer->inverseOff();
}

void UniPrinter::loop() { }

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
