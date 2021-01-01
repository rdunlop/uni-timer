#ifndef UNI_SENSOR_H
#define UNI_SENSOR_H

class UniSensor
{
  public:
    UniSensor(int input);
    void setupInterruptHandler(void (*interrupt_handler)());
    void loop();
    bool blocked();
    void attach_interrupt();
    void detach_interrupt();
  private:
    int _input;
    void (*_interrupt_handler)();
    bool _last_sensor;
};

#endif
