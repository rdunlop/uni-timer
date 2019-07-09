#ifndef UNI_SENSOR_H
#define UNI_SENSOR_H

class UniSensor
{
  static void sensor_interrupt(); // registered interrupt handler with the system
  static UniSensor *instance0_; // for use by interrupt handler
  
  public:
    UniSensor(int input);
    void setup();
    bool blocked();
    bool blocked_via_interrupt();
    void handle_interrupt();
    void attach_interrupt();
    void detach_interrupt();
    unsigned long interrupt_micros();
    void clear_interrupt_micros();
  private:
    int _input;
    unsigned long _interrupt_micros;
};

#endif
