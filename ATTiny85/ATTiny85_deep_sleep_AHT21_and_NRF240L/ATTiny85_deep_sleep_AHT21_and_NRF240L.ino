#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "RF24_mod.h"
#include <Wire.h>
#include <AHT20.h>
AHT20 aht20;

// As per implementation in RF24.cpp, set CE_PIN and CSN_PIN equal to
// use ATTiny85 3 pin mode.
#define CE_PIN  PB3
#define CSN_PIN PB3

// #define DEBUG_LEDS

// #define CE_PIN  PB3
// #define CSN_PIN PB4

const byte slaveAddress[5] = {'3','S','T','U','D'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

struct aht21Data {
  float temperature;
  float humidity;
} aht21Data;

unsigned long counter;
unsigned int steps;
const unsigned int sleep_milliseconds = 2228;

ISR(WDT_vect) {
  cli();
  wdt_disable();
  sei();
}

void set_sleep_time_in_minutes(unsigned int minutes) {
  steps = round((float)(minutes * 60) / (float)sleep_milliseconds * 1000.0);
}

void set_sleep_time_in_seconds(unsigned int seconds) {
  steps = round((float)seconds * 1000.0 / (float)sleep_milliseconds);
}

void myWatchdogEnable() {  // turn on watchdog timer; interrupt mode every 2.228 s; uses 4.5 uA
  cli();
  MCUSR = 0;
  WDTCR |= B00011000;
  WDTCR = B01001111;
  sei();
}

void deep_sleep() {
  ADCSRA &= ~(1<<ADEN); //Disable ADC, saves ~230 uA
  wdt_reset();
  myWatchdogEnable();
  sleep_mode();
  ADCSRA |= (1<<ADEN); //Enable ADC
}

void send() {
  // Always use sizeof() as it gives the size as the number of bytes.
  // For example if dataToSend was an int sizeof() would correctly return 2
  radio.write( &aht21Data, sizeof(aht21Data) );
}

void setup() {
  aht21Data.temperature = 0.0;
  aht21Data.humidity = 0.0;
#ifndef DEBUG_LEDS
  set_sleep_time_in_minutes(10);
#else
  set_sleep_time_in_seconds(2);
#endif
  counter = steps;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel(RF24_PA_HIGH);
  radio.setPayloadSize(sizeof(aht21Data));
  radio.setRetries(3,5); // delay, count
  radio.openWritingPipe(slaveAddress);
  radio.powerDown(); // Consumes 2.25 uA, 1.3 uA if CE and CSN connected to attiny

#ifndef DEBUG_LEDS
  Wire.begin();
  aht20.begin();
#else
  pinMode(PB0, OUTPUT);
  pinMode(PB2, OUTPUT);

  digitalWrite(PB2, HIGH);
  delay(500);
  digitalWrite(PB2, LOW);
  delay(500);
#endif
}

void loop() {
  if (counter < steps) {
    counter++;
    deep_sleep();
    return;
  }

  aht21Data.temperature = -1.0;
  aht21Data.humidity = -1.0;

#ifndef DEBUG_LEDS
  aht21Data.temperature = aht20.getTemperature();
  aht21Data.humidity = aht20.getHumidity();
#else
  digitalWrite(PB0, HIGH);
  delay(200);
  digitalWrite(PB0, LOW);
  delay(200);
#endif
  counter = 0;
  radio.powerUp();
  send();  
  radio.powerDown();
  // Total consumption when sleeping 13 uA
}