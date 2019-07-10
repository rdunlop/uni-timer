#ifndef UNI_SENSOR_H
#define UNI_SENSOR_H

class UniSensor
{
  public:
    UniSensor(int input);
    void setup(void (*interrupt_handler)());
    bool blocked();
    void attach_interrupt();
    void detach_interrupt();
  private:
    int _input;
    void (*_interrupt_handler)();
};

#endif
