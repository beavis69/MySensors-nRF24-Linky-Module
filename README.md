# MySensors nRF24 Linky Module
IoT energy sensor for Linky electricity meters in France

It's a fork from https://github.com/sitronlabs/MySensors-nRF24-Linky-Module
using modified version of https://github.com/sitronlabs/SitronLabs_Enedis_TIC_Arduino_Library

I works only for Linky *standard tic at 9600b/s*

It only reports the following data :

*    SENSOR_0_SERIAL,         // S_INFO       (V_TEXT)    ADSC
*    SENSOR_1_CURRENT,        // S_MULTIMETER (V_CURRENT) IRMS1
*    SENSOR_2_VOLTAGE,        // S_MULTIMETER (V_CURRENT) URMS1
*    SENSOR_3_POWER,          // S_POWER      (V_WATT)    SINSTS
*    SENSOR_4_TOTAL,          // S_POWER      (V_KWH)     EAST


Some notes :
* use less ram (discarding unused dataset)
* tic leds are disabled for more stability
* nrf24 leds blink period is modified to 20ms (default is 300ms) for stability

