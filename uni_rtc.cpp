// - RTC
#include "uni_rtc.h"

UniRtc::UniRtc(int sqw_signal_input)
{
  _sqw_signal_input = sqw_signal_input;
}

// Because interrupt handlers run outside of all object-like constructs
// we have a static pointer to a GPS instance, so that we can bounce into
// the interrupt_handler on that object
UniRtc * UniRtc::instance0_;

// RTC SQW signal is an active-low signal, so it needs INPUT_PULLUP
void UniRtc::sqw_interrupt(){
  instance0_->handle_interrupt();
}

void UniRtc::handle_interrupt() {
  rtc_interrupt_flag = true;
  Serial.print("RTC: ");
  unsigned long  now = micros();
  Serial.println(now - rtc_start_ms);
  rtc_start_ms = now;
}

void UniRtc::setup() {
  Serial.println("RTC Initializing");
  rtc_interrupt_flag = false;
  rtc_start_ms = micros();
  last_rtc_print_time = millis();
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // I don't know if this does anything
  // enable the 1 Hz output
  //rtc.writeSqwPinMode (DS3231_SquareWave1Hz);

  instance0_ = this;
  pinMode(_sqw_signal_input, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_sqw_signal_input), sqw_interrupt, RISING);
  Serial.println("Done RTC Initialization");
}

void UniRtc::loop() {
// we can NOT call delay here.
//  if (rtc_interrupt_flag) {
//    digitalWrite(LED_BUILTIN, HIGH);    // flash the led
////    delay(100);                         // wait a little bit
//    digitalWrite(LED_BUILTIN, LOW);     // turn off led
//    rtc_interrupt_flag =  false;                      // clear the flag until timer sets it again
//  }
}

void UniRtc::printPeriodically() {

  // if millis() or timer wraps around, we'll just reset it
  if (last_rtc_print_time > millis())  last_rtc_print_time = millis();
  // approximately every 2 seconds or so, print out the current GPS stats
  if (millis() - last_rtc_print_time > 2000) { 
    last_rtc_print_time = millis(); // reset the timer
    print();
  }
}

void UniRtc::setDateTime(int year, int month, int day, int hour, int minute, int second) {
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
}

DateTime UniRtc::getDateTime() {
  return rtc.now();
}
      

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
void UniRtc::print() {
  DateTime now = rtc.now();

  Serial.println("---------------------------");
  Serial.println("RTC:");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");
  
  // calculate a date which is 7 days and 30 seconds into the future
  DateTime future (now + TimeSpan(7,12,30,6));
  
  Serial.print(" now + 7d + 30s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();
  
  Serial.println();
}
