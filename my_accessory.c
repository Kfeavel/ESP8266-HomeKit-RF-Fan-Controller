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
  // TODO: Blink the light
  printf("TODO: Identify the ceiling fan light\n");
}

homekit_characteristic_t cha_light_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_light_name = HOMEKIT_CHARACTERISTIC_(NAME, "Light");

homekit_characteristic_t cha_fan_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_fan_speed = HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 0);
homekit_characteristic_t cha_fan_name = HOMEKIT_CHARACTERISTIC_(NAME, "Fan");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_fan, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
          HOMEKIT_CHARACTERISTIC(NAME, "Ceiling Fan"),
          HOMEKIT_CHARACTERISTIC(MANUFACTURER, "AI Thinker"),
          HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "MJBPJ-CMTHH-8YHX5"),
          HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266MOD"),
          HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
          HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
          NULL
      }),
      HOMEKIT_SERVICE(FAN, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
        &cha_fan_on,
        &cha_fan_speed,
        &cha_fan_name,
        NULL
      }),
      NULL
    }),
    HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
          HOMEKIT_CHARACTERISTIC(NAME, "Ceiling Fan Light"),
          HOMEKIT_CHARACTERISTIC(MANUFACTURER, "AI Thinker"),
          HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "MJBPJ-CMTHH-8YHX5"),
          HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266MOD"),
          HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
          HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
          NULL
      }),
      HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
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
    .password = "111-11-111"
};


