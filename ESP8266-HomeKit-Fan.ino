/*
 * ESP8266-HomeKit-Fan.ino
 */

#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static RCSwitch rf315Switch = RCSwitch();

//==============================
// Arduino
//==============================

void setup() {
  // ESP8266 setup
  Serial.begin(9600);
  ets_update_cpu_frequency(160);
  pinMode(LED_BUILTIN, OUTPUT);

  // Check for reset line
  pinMode(4, INPUT); // D2
  if (digitalRead(4) == HIGH) {
    LOG_D("Resetting HomeKit storage!");
    homekit_storage_reset();
    for (size_t i = 10; i > 0; i--) {
      LOG_D("Resetting in %d seconds...", i);
      delay(1000);
    }
    
    ESP.reset();
  }
  
  // 315 Mhz transmitter setup
  rf315Switch.enableTransmit(10); // SD3
  rf315Switch.setProtocol(6);
  rf315Switch.setPulseLength(400);
  rf315Switch.setRepeatTransmit(10);

  // HomeKit setup
  wifi_connect();
  my_homekit_setup();

  // Reset LED
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {
  arduino_homekit_loop();
}

//==============================
// RF 315 Mhz Helpers
//==============================

static void transmitRFData(unsigned long data, unsigned int len) {
  rf315Switch.send(data, len);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_light_on;
extern "C" homekit_characteristic_t cha_fan_on;
extern "C" homekit_characteristic_t cha_fan_speed;

static uint32_t next_heap_millis = 0;

// Called when the switch value is changed by iOS Home APP
static void cha_light_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);

  bool on = value.bool_value;
  if (cha_light_on.value.bool_value != on) {
    cha_light_on.value.bool_value = on;  // sync the value

    LOG_D("[SET] Light: %s", on ? "ON" : "OFF");
    transmitRFData(0xBF9, 12);
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_light_on_getter() {
  LOG_D("[GET] Light on: %s", (cha_light_on.value.bool_value ? "ON" : "OFF"));
  return cha_light_on.value;
}

static void cha_fan_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);

  bool on = value.bool_value;
  if (cha_fan_on.value.bool_value != on) {
    cha_fan_on.value.bool_value = on;  // sync the value
    
    LOG_D("[SET] Fan: %s", on ? "ON" : "OFF");
    transmitRFData(0xFB9, 12);
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_fan_on_getter() {
  LOG_D("[GET] Fan: %s", (cha_fan_on.value.bool_value ? "ON" : "OFF"));
  return cha_fan_on.value;
}

static void cha_fan_speed_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);

  float speed = value.float_value;
  if (cha_fan_speed.value.float_value != speed) {
    cha_fan_speed.value.float_value = speed;  // sync the value

    LOG_D("[SET] Speed: %f", speed);
    if (speed == 0) {
      // Turn off fan
      transmitRFData(0xFB9, 12);
    } else if (speed > 0 && speed < 33) {
      transmitRFData(0x7F9, 12);
    } else if (speed > 33 && speed < 66) {
      transmitRFData(0xEF9, 12);
    } else if (speed > 66 && speed < 100) {
      transmitRFData(0xF79, 12);
    }

    cha_fan_on.value.bool_value = (speed > 0);
    homekit_characteristic_notify(&cha_fan_on, cha_fan_on.value);
    LOG_D("[NOTIFY] Fan: %s", (cha_fan_on.value.bool_value ? "ON" : "OFF"));
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_fan_speed_getter() {
  LOG_D("[GET] Fan speed: %f", cha_fan_speed.value.float_value);
  return cha_fan_speed.value;
}

static void my_homekit_setup() {
  cha_light_on.setter = cha_light_on_setter;
  cha_light_on.getter = cha_light_on_getter;
  cha_fan_on.setter = cha_fan_on_setter;
  cha_fan_on.getter = cha_fan_on_getter;
  cha_fan_speed.setter = cha_fan_speed_setter;
  cha_fan_speed.getter = cha_fan_speed_getter;
  arduino_homekit_setup(&config);
}

