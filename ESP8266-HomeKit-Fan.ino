/*
 * ESP8266-HomeKit-Fan.ino
 */

#include <Arduino.h>
#include <RCSwitch.h>
#include <EasyButton.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static RCSwitch rf315Switch = RCSwitch();

static void transmitRFData(unsigned long data, unsigned int len);

//==============================
// Arduino
//==============================

void setup() {
  // ESP8266 setup
  Serial.begin(9600);
  ets_update_cpu_frequency(160);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // 315 Mhz transmitter setup
  // rf315Switch.enableReceive(4); // D2
  rf315Switch.enableTransmit(10); // SD3
  rf315Switch.setProtocol(6);
  rf315Switch.setPulseLength(400);
  rf315Switch.setRepeatTransmit(10);

  // homekit_storage_reset();

  // HomeKit setup
  wifi_connect();
  my_homekit_setup();
}

void loop() {
  arduino_homekit_loop();
  delay(10);
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
extern "C" homekit_characteristic_t cha_fan_active;
extern "C" homekit_characteristic_t cha_fan_speed;

static uint32_t next_heap_millis = 0;

// Called when the switch value is changed by iOS Home APP
static void cha_light_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);
  bool on = value.bool_value;
  cha_light_on.value.bool_value = on;  // sync the value

  LOG_D("Light: %s", on ? "ON" : "OFF");
  transmitRFData(0xBF9, 12);
  digitalWrite(BUILTIN_LED, HIGH);
}

static void cha_fan_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);
  bool on = value.bool_value;
  cha_fan_active.value.bool_value = on;  // sync the value
  
  LOG_D("Fan: %s", on ? "ON" : "OFF");
  transmitRFData(0xFB9, 12);
  digitalWrite(BUILTIN_LED, HIGH);
}

static void cha_fan_speed_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);
  float speed = value.float_value;
  cha_fan_speed.value.float_value = speed;  // sync the value

  LOG_D("Speed: %f", speed);
  if (speed > 0 && speed < 33) {
    transmitRFData(0x7F9, 12);
  } else if (speed > 33 && speed < 66) {
    transmitRFData(0xEF9, 12);
  } else if (speed > 66 && speed < 100) {
    transmitRFData(0xF79, 12);
  }

  digitalWrite(BUILTIN_LED, HIGH);
  //cha_switch_on.value.bool_value = switch_is_on;
  //homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

static void my_homekit_setup() {
  // Add the .setter function to get the switch-event sent from iOS Home APP.
  // The .setter should be added before arduino_homekit_setup.
  cha_light_on.setter = cha_light_on_setter;
  cha_fan_active.setter = cha_fan_on_setter;
  cha_fan_speed.setter = cha_fan_speed_setter;
  arduino_homekit_setup(&config);
}

//==============================
// Debugging
//==============================

static void debug_print_heap() {
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
        ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}
