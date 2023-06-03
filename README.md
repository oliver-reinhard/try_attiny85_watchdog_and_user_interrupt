# try_attiny85_watchdog_and_user_interrupt
Demo of watdchdog and external interrupts for Arduino Uno and AVR ATtiny85

Demonstrates the use of
  - external interrupt INT0 (Arduino Uno / ATmega328) and INT interrupt (ATtiny85), respectively
  - pin-change interrupt PCINT0 on both ATmega328 and ATtiny85
  - watchdog interrupts (every 8 seconds) on both ATmega328 and ATtiny85
to wake up MCU from sleep.

Like this, the MCU spends most of its time sleeping and only uses real energy when doing useful work -> turning on and off two LEDs:
 - green when watchdog time-out occurs
 - yellow when sleep is interrupted.

Note that a watchdog interrupt also interrupts sleep.

INT0 / INT is triggered via a dedicated manual switch -> creates an interrupt only on press, not on release
PCINT0 is triggered via a dedicated manual switch -> creates an interrupt on both press and release

See the base for this project in this repository: https://github.com/oliver-reinhard/attiny85-watchdog 

Expected behaviour of this Arduino sketch on MCU reset:
  1. Green LED lights up solid for 1 second
  2. Yellow LED lights up solid for 1 second, then:
  3. Watchdog goes off every 8 second and triggers an interrupt:
     1. green blink for 200 ms (= watchdog time-out)
     2. yellow blink for 200 ms (= sleep interrupt)
  - On pressing INT0 / INT switch:
    -> yellow blink for 200 ms (= sleep interrupt) 
  - On releasing INT0 / INT switch:
    -> (nothing)
  - On pressing PCINT0 switch:
    -> yellow blink for 200 ms (= sleep interrupt)  
  - On releasing PCINT0 switch:
    -> yellow blink for 200 ms (= sleep interrupt) 

Oliver Reinhard, 2023
