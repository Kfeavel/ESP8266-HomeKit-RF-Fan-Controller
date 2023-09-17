/*
 * ESP8266-HomeKit-Fan.ino
 */

#include <Arduino.h>
#include <RCSwitch.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define RF_RX_GPIO  0
#define RF_TX_GPIO  10

// Fan Remote Controlle RF Values
#define FAN_RF_DATA_LEN 12
#define FAN_RD_DATA_SPEED_LOW     0x7F9
#define FAN_RD_DATA_SPEED_MED     0xEF9
#define FAN_RD_DATA_SPEED_HIGH    0xF79
#define FAN_RD_DATA_FAN_TOGGLE    0xFB9
#define FAN_RD_DATA_LIGHT_TOGGLE  0xBF9

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_light_on;
extern "C" homekit_characteristic_t cha_fan_on;
extern "C" homekit_characteristic_t cha_fan_speed;

static RCSwitch rf315Switch = RCSwitch();

//==============================
// Arduino
//==============================

void setup(void) {
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
  rf315Switch.enableReceive(RF_RX_GPIO);  // D3
  rf315Switch.enableTransmit(RF_TX_GPIO); // SD3
  rf315Switch.setProtocol(6);
  rf315Switch.setPulseLength(400);
  rf315Switch.setRepeatTransmit(10);

  // HomeKit setup
  wifi_connect();
  my_homekit_setup();

  // Reset LED
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop(void) {
  arduino_homekit_loop();
  receive_rf_update();
}

//==============================
// RF 315 Mhz Helpers
//==============================

static void external_fan_update(float speed) {
    cha_fan_speed.value.float_value = speed;
    homekit_characteristic_notify(&cha_fan_speed, cha_fan_speed.value);

    cha_fan_on.value.bool_value = (speed > 0);
    homekit_characteristic_notify(&cha_fan_on, cha_fan_on.value);
}

static void receive_rf_update(void) {
  if (rf315Switch.available()) {
    digitalWrite(BUILTIN_LED, LOW);

    unsigned long value = rf315Switch.getReceivedValue();
    switch (value) {
      case FAN_RD_DATA_SPEED_LOW:
        external_fan_update(25.0f);
        break;

      case FAN_RD_DATA_SPEED_MED:
        external_fan_update(50.0f);
        break;

      case FAN_RD_DATA_SPEED_HIGH:
        external_fan_update(75.0f);
        break;

      case FAN_RD_DATA_FAN_TOGGLE:
        // Fan power button was pressed -- only turns fan off if already on
        if (cha_fan_on.value.bool_value) {
          cha_fan_on.value.bool_value = false;
          homekit_characteristic_notify(&cha_fan_on, cha_fan_on.value);
          LOG_D("[SET] Fan on: %s", (cha_light_on.value.bool_value ? "ON" : "OFF"));
        }
        break;

      case FAN_RD_DATA_LIGHT_TOGGLE:
        // Light power button was pressed -- toggle state
        cha_light_on.value.bool_value = ! cha_light_on.value.bool_value;
        homekit_characteristic_notify(&cha_light_on, cha_light_on.value);
        LOG_D("[SET] Light on: %s", (cha_light_on.value.bool_value ? "ON" : "OFF"));
        break;

      default:
        break;
    }

    digitalWrite(BUILTIN_LED, HIGH);
    rf315Switch.resetAvailable();
  }
}

static void transmitRFData(unsigned long data, unsigned int len) {
  rf315Switch.disableReceive();
  rf315Switch.send(data, len);
  rf315Switch.resetAvailable();
  rf315Switch.enableReceive(RF_RX_GPIO);
}

//==============================
// HomeKit setup and loop
//==============================

static uint32_t next_heap_millis = 0;

// Called when the switch value is changed by iOS Home APP
static void cha_light_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);

  bool on = value.bool_value;
  if (cha_light_on.value.bool_value != on) {
    cha_light_on.value.bool_value = on;  // sync the value

    LOG_D("[SET] Light: %s", on ? "ON" : "OFF");
    transmitRFData(FAN_RD_DATA_LIGHT_TOGGLE, FAN_RF_DATA_LEN);
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_light_on_getter(void) {
  LOG_D("[GET] Light on: %s", (cha_light_on.value.bool_value ? "ON" : "OFF"));
  return cha_light_on.value;
}

static void cha_fan_on_setter(const homekit_value_t value) {
  digitalWrite(BUILTIN_LED, LOW);

  bool on = value.bool_value;
  if (cha_fan_on.value.bool_value != on) {
    cha_fan_on.value.bool_value = on;  // sync the value
    
    LOG_D("[SET] Fan: %s", on ? "ON" : "OFF");
    transmitRFData(FAN_RD_DATA_FAN_TOGGLE, FAN_RF_DATA_LEN);
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_fan_on_getter(void) {
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
      // TODO: Could this cause desync?
      transmitRFData(FAN_RD_DATA_FAN_TOGGLE, FAN_RF_DATA_LEN);
    } else if (speed > 0 && speed < 33) {
      transmitRFData(FAN_RD_DATA_SPEED_LOW, FAN_RF_DATA_LEN);
    } else if (speed > 33 && speed < 66) {
      transmitRFData(FAN_RD_DATA_SPEED_MED, FAN_RF_DATA_LEN);
    } else if (speed > 66 && speed < 100) {
      transmitRFData(FAN_RD_DATA_SPEED_HIGH, FAN_RF_DATA_LEN);
    }

    cha_fan_on.value.bool_value = (speed > 0);
    homekit_characteristic_notify(&cha_fan_on, cha_fan_on.value);
    LOG_D("[NOTIFY] Fan: %s", (cha_fan_on.value.bool_value ? "ON" : "OFF"));
  }

  digitalWrite(BUILTIN_LED, HIGH);
}

static homekit_value_t cha_fan_speed_getter(void) {
  LOG_D("[GET] Fan speed: %f", cha_fan_speed.value.float_value);
  return cha_fan_speed.value;
}

static void my_homekit_setup(void) {
  cha_light_on.setter = cha_light_on_setter;
  cha_light_on.getter = cha_light_on_getter;
  cha_fan_on.setter = cha_fan_on_setter;
  cha_fan_on.getter = cha_fan_on_getter;
  cha_fan_speed.setter = cha_fan_speed_setter;
  cha_fan_speed.getter = cha_fan_speed_getter;
  arduino_homekit_setup(&config);
}

