# try_attiny85_watchdog_and_user_interrupt
Demo of watdchdog and external interrupts for Arduino Uno and AVR ATtiny85

Applies to both Arduino Uno boards (AVR ATmega328) and AVR ATtiny85 chips, however, with a small variation for ATtiny85 with respect to INT0 interrupts.

Demonstrates the use of
  - external interrupt INT0 
  - pin-change interrupt PCINT0 
  - watchdog interrupts (every 8 seconds)
to wake up MCU from sleep.

Like this, the MCU spends most of its time sleeping and only uses real energy when doing useful work -> turning on and off two LEDs:
 - green when watchdog time-out occurs or while low-level INT on ATtiny is maintained
 - yellow when sleep is interrupted.

Note that a watchdog interrupt also interrupts sleep.

INT0 / INT is triggered via a dedicated manual switch:
    - ATmega328: configured to detect falling-edge input -> creates an interrupt only on press, not on release
    - ATtiny85: can only wake up with LOW-level input -> creates repeated interrupts while LOW level is maintained. AFTER input goes back to HIGH -> executes code following sleep_cpu() command.
    
PCINT0 is triggered via a dedicated manual switch -> creates an interrupt on both press and release.

See the base for this project in this repository: https://github.com/oliver-reinhard/attiny85-watchdog 

Expected behaviour of this Arduino sketch on MCU reset:
  1. Green LED lights up solid for 1 second
  2. Yellow LED lights up solid for 1 second, then:
  3. Watchdog goes times out every 8 second and triggers an interrupt:
     1. green blink for 200 ms (= watchdog time-out)
     2. yellow blink for 200 ms (= sleep interrupt)
  4. Manual interrupts:
    - On pressing INT0 switch (ATmega328) -> yellow blink for 200 ms (= sleep interrupt)
    - On releasing INT0 switch (ATmega328) -> (nothing)
    - On pressing INT switch (ATtiny85) -> very fast flashing of green LED (= continuous interrupts)
    - On releasing INT switch (ATtiny85) -> yellow blink for 200 ms (= sleep interrupt) 
    - On pressing PCINT0 switch -> yellow blink for 200 ms (= sleep interrupt)  
    - On releasing PCINT0 switch -> yellow blink for 200 ms (= sleep interrupt) 
  5. After 10 watchdog time-outs, the watchdog is disabled -> no more green blinks.
  6. Manual interrupts continue to work as described above.

Oliver Reinhard, 2023