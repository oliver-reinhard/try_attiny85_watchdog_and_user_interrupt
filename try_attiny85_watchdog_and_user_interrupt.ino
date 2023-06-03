/*
 * Demonstrates the use of
 *   - external interrupt INT0 (Arduino Uno / ATmega328) and INT interrupt (ATtiny85), respectively
 *   - pin-change interrupt PCINT0 on both ATmega328 and ATtiny85
 *   - watchdog interrupts (every 8 seconds) on both ATmega328 and ATtiny85
 * to wake up MCU from sleep.
 *
 * Like this, the MCU spends most of its time sleeping and only uses real energy 
 * when doing useful work -> turning on and off two LEDs:
 *  - green when watchdog time-out occurs
 *  - yellow when sleep is interrupted.
 *
 * Note that a watchdog interrupt also interrupts sleep.
 *
 * INT0 / INT is triggered via a dedicated manual switch -> creates an interrupt only on press, not on release
 * PCINT0 is triggered via a dedicated manual switch -> creates an interrupt on both press and release
 *
 * See the base for this project in this repository: 
 * https://github.com/oliver-reinhard/attiny85-watchdog 
 * 
 * Expected behaviour:
 * 
 * 1. MCU reset
 * 2. Green LED lights up solid for 1 second
 * 3. Yellow LED lights up solid for 1 second, then:
 * 4. Watchdog goes off every 8 second and triggers an interrupt
 *    1. green blink for 200 ms (= watchdog time-out)
 *    2. yellow blink for 200 ms (= sleep interrupt)
 * - On pressing INT0 / INT switch:
 *    - yellow blink for 200 ms (= sleep interrupt) 
 * - On releasing INT0 / INT switch:
 *    - (nothing)
 * - On pressing PCINT0 switch:
 *    - yellow blink for 200 ms (= sleep interrupt)  
 * - On releasing PCINT0 switch:
 *    - yellow blink for 200 ms (= sleep interrupt) 
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

int8_t watchdog_timeouts_remaining = 20;

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
  
  if (watchdog_timeouts_remaining == 0) {
    disable_watchdog();
  }
}

// This runs each time the watch dog wakes us up from sleep
ISR(WDT_vect) {
  watchdog_timeouts_remaining--;
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
    EICRA = _BV(ISC01);       // Only falling-edge change triggers an interrupt

  #elif defined(__AVR_ATtiny85__)
    GIMSK |= _BV(INT0);      // Enable INT0 (external interrupt) 
    MCUCR |= _BV(ISC01);     // Only falling-edge change triggers an interrupt
  #endif
}

ISR (INT0_vect) {       // Interrupt service routine for INT0 on PB2
  // do nothing --> this shold just interrupt sleep
  delay(50); // debounce switch (possibly unnecessary since this should turn an LED to ON for some time)
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
  // do nothing --> this shold just interrupt sleep
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
