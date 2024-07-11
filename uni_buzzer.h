#ifndef UNI_BUZZER_H
#define UNI_BUZZER_H

class UniBuzzer
{
  public:
    UniBuzzer(int output);
    void setup();
    void loop();
    void beep();
    void pre_beep();
    void start_beep();
    void success();
    void failure();
    void fault();
  private:
    int _output;
};

#endif
