/*
 * my_accessory.c
 * Define the accessory in C language using the Macro in characteristics.h
 *
 *  Created on: 2020-05-15
 *      Author: Mixiaoxiao (Wang Bin)
 */

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void light_identify(homekit_value_t _value) {
  printf("accessory identify\n");
}

void fan_identify(homekit_value_t _value) {
  printf("accessory identify\n");
}

// Light
// required: ON
// optional: NAME

// format: bool; HAP section 9.70; write the .setter function to get the switch-event sent from iOS Home APP.
homekit_characteristic_t cha_light_on = HOMEKIT_CHARACTERISTIC_(ON, false);
// format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_light_name = HOMEKIT_CHARACTERISTIC_(NAME, "Light");

// Fan (HAP ...)

// format: bool; HAP section 9.70; write the .setter function to get the switch-event sent from iOS Home APP.
homekit_characteristic_t cha_fan_on = HOMEKIT_CHARACTERISTIC_(ON, false);
// format: uint8_t
homekit_characteristic_t cha_fan_speed = HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 0.0);
// // format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_fan_name = HOMEKIT_CHARACTERISTIC_(NAME, "Fan");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_fan, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
          HOMEKIT_CHARACTERISTIC(NAME, "Fan"),
          HOMEKIT_CHARACTERISTIC(MANUFACTURER, "AI Thinker"),
          HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "MJBPJ-CMTHH-8YHX5"),
          HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266MOD"),
          HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
          HOMEKIT_CHARACTERISTIC(IDENTIFY, fan_identify),
          NULL
      }),
      HOMEKIT_SERVICE(FAN, .primary=true, .characteristics=(homekit_characteristic_t*[]){
        &cha_fan_on,
        &cha_fan_speed,
        &cha_fan_name,
        NULL
      }),
      NULL
    }),
    HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
          HOMEKIT_CHARACTERISTIC(NAME, "Light"),
          HOMEKIT_CHARACTERISTIC(MANUFACTURER, "AI Thinker"),
          HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "MJBPJ-CMTHH-8YHX5"),
          HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266MOD"),
          HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
          HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
          NULL
      }),
      HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
        &cha_light_on,
        &cha_light_name,
        NULL
      }),
      NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "514-18-136"
};


