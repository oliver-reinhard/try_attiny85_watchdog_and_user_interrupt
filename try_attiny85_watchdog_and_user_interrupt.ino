/*
 * Applies to both Arduino Uno boards (AVR ATmega328) and AVR ATtiny85 chips, however, with a small variation 
 * for ATtiny85 with respect to INT0 interrupts.
 *
 * Demonstrates the use of
 *   - external interrupt INT0
 *   - pin-change interrupt PCINT0
 *   - watchdog interrupts (every 8 seconds) on both ATmega328 and ATtiny85
 * to wake up MCU from sleep.
 *
 * Like this, the MCU spends most of its time sleeping and only uses real energy 
 * when doing useful work -> turning on and off two LEDs:
 *  - green when watchdog time-out occurs or while low-level INT on ATtiny is maintained
 *  - yellow when sleep is interrupted.
 *
 * Note that a watchdog interrupt also interrupts sleep.
 *
 * INT0 is triggered via a dedicated manual switch:
 *  - ATmega328: configured to detect falling-edge input -> creates an interrupt only on press, not on release
 *  - ATtiny85: can only wake up with LOW-level input -> creates repeated interrupts while LOW level is maintained. 
 *    After input goes back to HIGH -> executes code following sleep_cpu() command.
 *
 * PCINT0 is triggered via a dedicated manual switch -> creates an interrupt on both press and release.
 *
 * ==> As a consequence, INT0 is best no used on ATtiny85 in combination with sleep_cpu(). Use PCINT0 instead and trigger 
 * actions depending on current PCINTx port input values and/or port input changes (HIGH to LOW or LOW to HIGH).
 *
 *
 * See the base for this project in this repository: 
 * https://github.com/oliver-reinhard/attiny85-watchdog 
 * 
 * Expected behaviour on MCU reset:
 * 1. Green LED lights up solid for 1 second
 * 2. Yellow LED lights up solid for 1 second, then:
 * 3. Watchdog times out every 8 second and triggers an interrupt
 *    1. green blink for 200 ms (= watchdog time-out)
 *    2. yellow blink for 200 ms (= sleep interrupt)
 * 4. Manual interrupts:
 *    - On pressing INT0 switch (ATmega328) -> yellow blink for 200 ms (= sleep interrupt)
 *    - On releasing INT0 switch (ATmega328) -> (nothing)
 *    - On pressing INT switch (ATtiny85) -> very fast flashing of green LED (= continuous interrupts)
 *    - On releasing INT switch (ATtiny85) -> yellow blink for 200 ms (= sleep interrupt) 
 *    - On pressing PCINT0 switch -> yellow blink for 200 ms (= sleep interrupt)  
 *    - On releasing PCINT0 switch -> yellow blink for 200 ms (= sleep interrupt) 
 * 5. After 10 watchdog time-outs, the watchdog is disabled -> no more green blinks.
 * 6. Manual interrupts continue to work as described above.
 * 
 * Oliver Reinhard, 2023
 */
#include <avr/sleep.h>
#include <avr/wdt.h>

#if defined(__AVR_ATmega328P__)
  const uint8_t WATCHDOG_TIMEOUT_LED_PIN = 13;      // digital: out LED — PB5 is the built-in on ATmega328 board)
  const uint8_t SLEEP_INTERRUPT_LED_PIN = 12;       // digital: out 
  const uint8_t INT0_PIN = 2;                       // external-interrupt source; only pin 2 will trigger INT0
  const uint8_t PCINT0_PIN = 8;                     // pin-change interrupt source 

#elif defined(__AVR_ATtiny85__)
  const uint8_t WATCHDOG_TIMEOUT_LED_PIN = PB0;     // digital: out (LED — PB0 is the built-in on ATtiny85 programmer)
  const uint8_t SLEEP_INTERRUPT_LED_PIN = PB1;      // digital: out 
  const uint8_t INT0_PIN = PB2;                     // external-interrupt source; only pin PB2 will trigger INT0
  const uint8_t PCINT0_PIN = PB3;                   // pin-change interrupt source
#endif

int8_t watchdogTimeoutsRemaining = 10;

void setup() {
  disable_watchdog();

  pinMode(WATCHDOG_TIMEOUT_LED_PIN, OUTPUT);
  pinMode(SLEEP_INTERRUPT_LED_PIN, OUTPUT);

  configInputWithPullup(INT0_PIN);  // Pushbutton --> INT0
  configInt0();
  configInputWithPullup(PCINT0_PIN);  // Pushbutton --> PCINT0
  configPinChangeInt0();

  turnOnLED(WATCHDOG_TIMEOUT_LED_PIN, 1000);
  turnOnLED(SLEEP_INTERRUPT_LED_PIN, 1000);

  wdt_enable(WDTO_8S); // 8 seconds
  _WD_CONTROL_REG |= _BV(WDIE);  // Must also reset WDIE after each interrupt!!
  sei();
}

void loop() {
  //  sleep_mode(); // Blocking sleep ("idle" mode by default)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sei();
  sleep_cpu();      // Controller waits for interrupt here
  sleep_disable();
  flashLED(SLEEP_INTERRUPT_LED_PIN, 1); 
  
  if (watchdogTimeoutsRemaining == 0) {
    disable_watchdog();
  }
}

// This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  watchdogTimeoutsRemaining--;
  flashLED(WATCHDOG_TIMEOUT_LED_PIN, 1); 
  _WD_CONTROL_REG |= _BV(WDIE);
}

void disable_watchdog() {
  MCUSR &= ~_BV(WDRF); // see comment in ATtiny85 Datasheet, p.46, Note under "Bit 3 – WDE: Watchdog Enable" 
  wdt_disable();
}

void configInt0() {
  #if defined(__AVR_ATmega328P__)
    EIMSK = _BV(INT0);        // Enable INT0 (external interrupt) 
    EICRA = _BV(ISC01);       // Only falling-edge change should trigger an interrupt

  #elif defined(__AVR_ATtiny85__)
    GIMSK |= _BV(INT0);      // Enable INT0 (external interrupt) 
    MCUCR &= ~(_BV(ISC01) | _BV(ISC00));     // Only LOW-level input will generate an interrupt when MCU is in sleep.
  #endif
}

ISR (INT0_vect) {       // Interrupt service routine for INT0 on PB2
  // Do nothing --> this shold just interrupt sleep
  // HOWEVER, this interrupt handler will execute FIRST and REPEATEDLY AS LONG AS LOW level is maintained;
  // Only when input returns to HGH LEVEL is any code following the sleep_cpu() command executed.
  //
  flashLED(WATCHDOG_TIMEOUT_LED_PIN, 2); 
}

void configPinChangeInt0() {
  // Pin-change interrupts are triggered for each level-change; this cannot be configured
  #if defined(__AVR_ATmega328P__)
    PCICR  |= _BV(PCIE0);          // Enable pin-change interrupt 0 
    PCMSK0 |= _BV(PCINT1);         // Configure pin 8 as pin-change source for PCINT interrupt

  #elif defined(__AVR_ATtiny85__)
    GIMSK|= _BV(PCIE);            // Enable pin-change interrupt
    PCMSK|= _BV(PCINT3);          // Configure PB3 as pin-change source for PCINT0 interrupt
  #endif
}

ISR (PCINT0_vect) {       // Interrupt service routine for Pin-Change Interrupt Request 0
  // Do nothing --> this shold just interrupt sleep
  // HOWEVER, this interrupt handler will execute FIRST and before any code following the sleep_cpu() command.
  delay(50); // debounce switch (possibly unnecessary since this should turn an LED to ON for some time)
}

void configInputWithPullup(uint8_t pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); // Activate pull-up resistor on input pin
}

void turnOnLED(uint8_t pin, uint32_t ms) {
  digitalWrite(pin, HIGH);
  delay(ms);
  digitalWrite(pin, LOW);
}
  
void flashLED(uint8_t pin, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(200);
    digitalWrite(pin, LOW);
    delay(200);
  }
}
