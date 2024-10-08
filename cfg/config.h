#ifndef CONFIG_H
#define CONFIG_H

/* MySensors configuration */
// #define MY_DEBUG
#define MY_RADIO_RF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_DEFAULT_ERR_LED_PIN A0
#define MY_DEFAULT_TX_LED_PIN A1
#define MY_DEFAULT_LED_BLINK_PERIOD   20
#define MY_RF24_CHANNEL 84
#define MY_SPLASH_SCREEN_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS 10000

/* Tic configuration */
#define CONFIG_TIC_DATA_PIN 2
#define CONFIG_TIC_DUMMY_PIN 5

/* Leds configuration */
#define CONFIG_LED_TIC_GREEN_PIN 4
#define CONFIG_LED_TIC_RED_PIN 3

/* debug tic */
#define CONFIG_TIC_DEBUG_ENABLED 0

#endif
