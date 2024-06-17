# MySensors nRF24 Linky Module
IoT energy sensor for Linky electricity meters in France

It's a fork from https://github.com/sitronlabs/MySensors-nRF24-Linky-Module
using modified version of https://github.com/sitronlabs/SitronLabs_Enedis_TIC_Arduino_Library

I works only for Linky *standard tic at 9600b/s*

It only reports the following data :

* S_INFO       (V_TEXT)    ADSC   : Serial
* S_MULTIMETER (V_CURRENT) IRMS1  : Current
* S_MULTIMETER (V_CURRENT) URMS1  : Voltage
* S_POWER      (V_VA)      SINSTS : Apparent Power
* S_POWER      (V_KWH)     EAST   : Energy Total
* S_POWER      (V_KWH)     CCASN  : Energy last hour
* S_POWER      (V_VA)      SMAXSN : Apparent Power Max
* S_POWER      (V_VA)      SINSTI : Apparent Power Injected
* S_POWER      (V_VA)      SMAXSI : Apparent Power Injected Max

Some notes :
* use less ram (discarding unused dataset)
* tic leds are disabled for more stability
* nrf24 leds blink period is modified to 20ms (default is 300ms) for stability

