/*
 * switch.ino
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 *
 * HAP section 8.38 Switch
 * An accessory contains a switch.
 *
 * This example shows how to:
 * 1. define a switch accessory and its characteristics (in my_accessory.c).
 * 2. get the switch-event sent from iOS Home APP.
 * 3. report the switch value to HomeKit.
 *
 * You should:
 * 1. read and use the Example01_TemperatureSensor with detailed comments
 *    to know the basic concept and usage of this library before other examples。
 * 2. erase the full flash or call homekit_storage_reset() in setup()
 *    to remove the previous HomeKit pairing storage and
 *    enable the pairing with the new accessory of this new HomeKit example.
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <RCSwitch.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static RCSwitch rf315Switch = RCSwitch();

void setup() {
  // ESP8266 setup
  Serial.begin(115200);
  ets_update_cpu_frequency(160);

  // RF 315 Mhz receiver setup
  rf315Switch.enableReceive(16);
  
  // HomeKit setup
  wifi_connect();
  // homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  my_homekit_setup();
}

void loop() {
  rf315_loop();
  arduino_homekit_loop();
  delay(10);
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

//Called when the switch value is changed by iOS Home APP
static void cha_light_on_setter(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_light_on.value.bool_value = on;  //sync the value
  LOG_D("Light: %s", on ? "ON" : "OFF");
}

static void cha_fan_on_setter(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_fan_on.value.bool_value = on;  //sync the value
  LOG_D("Fan: %s", on ? "ON" : "OFF");
}

static void cha_fan_speed_setter(const homekit_value_t value) {
  uint8_t speed = value.float_value;
  cha_fan_speed.value.float_value = speed;  //sync the value
  LOG_D("Speed: %f", speed);
}

static void my_homekit_setup() {
  //Add the .setter function to get the switch-event sent from iOS Home APP.
  //The .setter should be added before arduino_homekit_setup.
  //HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
  //Maybe this is a legacy design issue in the original esp-homekit library,
  //and I have no reason to modify this "feature".
  cha_light_on.setter = cha_light_on_setter;
  cha_fan_on.setter = cha_fan_on_setter;
  cha_fan_speed.setter = cha_fan_speed_setter;
  arduino_homekit_setup(&config);

  //report the switch value to HomeKit if it is changed (e.g. by a physical button)
  //bool switch_is_on = true/false;
  //cha_switch_on.value.bool_value = switch_is_on;
  //homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

static void rf315_loop() {
  if (rf315Switch.available()) {
    unsigned long value = rf315Switch.getReceivedValue();

    digitalWrite(LED_BUILTIN, LOW);
    LOG_D("Recieved value: 0x%X", value);
    digitalWrite(LED_BUILTIN, HIGH);

    rf315Switch.resetAvailable();
  }
}

static void debug_print_heap() {
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
        ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}
