#include "serial_protocol.h"
const int PIN_LED_BLUE = 2;
const int PIN_LED_RED = 3;
const int PIN_LED_YELLOW = 4;
const int PIN_LED_GREEN = 5;

uint8_t leds[4] = {
  PIN_LED_BLUE,
  PIN_LED_RED,
  PIN_LED_YELLOW,
  PIN_LED_GREEN,
};


void setup()
{
  // Set led pins to output
  for (int i = 0; i < sizeof(leds); ++i)
    pinMode(leds[i], OUTPUT);

  Serial.begin(9600);
}

void loop()
{
  if (Serial.available() >= 2) {
    uint8_t cmd = Serial.read();
    uint8_t arg = Serial.read();
    execute(cmd, arg);
  }
}

void execute(uint8_t cmd, uint8_t arg)
{
  if (arg >= sizeof(leds)) {
    write_string("Invalid argument");
    return;
  }
  switch (cmd) {
  case Command::On: {
    digitalWrite(leds[arg], 1);
    write_string("OK");
    break;
  }
  case Command::Off: {
    digitalWrite(leds[arg], 0);
    write_string("OK");
    break;
  }
  case Command::Toggle: {
    digitalWrite(leds[arg], !digitalRead(leds[arg]));
    write_string("OK");
    break;
  }
  case Command::State: {
    bool value = digitalRead(leds[arg]);
    write_string(value ? "ON" : "OFF");
    break;
  }
  default:
    write_string("Invalid command");
    break;
  }
}

void write_string(const char *text)
{
  Serial.write(text);
  Serial.write(0);
}
