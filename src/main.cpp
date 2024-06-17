/* Config */
#include "../cfg/config.h"

/* Arduino Libraries */
#include <Arduino.h>
#include <MySensors.h>
#include <SoftwareSerial.h>
#include <tic_reader.h>

/* C/C++ libraries */
#include <ctype.h>
#include <stdlib.h>


// define force refresh period
#define PERIOD 1800000

/* Working variables */
static SoftwareSerial m_tic_port(CONFIG_TIC_DATA_PIN, CONFIG_TIC_DUMMY_PIN);
static tic_reader m_tic_reader;
static enum {
    STATE_STARTING,
    STATE_VALID,
    STATE_INVALID,
} m_tic_state;

/* List of virtual sensors */
// S_INFO       (V_TEXT)    ADSC   : Serial
// S_MULTIMETER (V_CURRENT) IRMS1  : Current
// S_MULTIMETER (V_CURRENT) URMS1  : Voltage
// S_POWER      (V_VA)      SINSTS : Apparent Power
// S_POWER      (V_KWH)     EAST   : Energy Total
// S_POWER      (V_KWH)     CCASN  : Energy last hour
// S_POWER      (V_VA)      SMAXSN : Apparent Power Max
// S_POWER      (V_VA)      SINSTI : Apparent Power Injected
// S_POWER      (V_VA)      SMAXSI : Apparent Power Injected Max
enum {
    SENSOR_0_SERIAL,
    SENSOR_1_CURRENT,
    SENSOR_2_VOLTAGE,
    SENSOR_3_POWER,
    SENSOR_4_TOTAL,
    SENSOR_5_LAST_H,
    SENSOR_6_POWER_MAX,
    SENSOR_7_POWER_IN,
    SENSOR_8_POWER_IN_MAX,
};

/**
 * Setup function.
 * Called before MySensors does anything.
 */
void preHwInit(void) {
    /* Setup leds
     * Ensures tic link led is off at startup */
    pinMode(CONFIG_LED_TIC_GREEN_PIN, OUTPUT);
    pinMode(CONFIG_LED_TIC_RED_PIN, OUTPUT);
    digitalWrite(CONFIG_LED_TIC_GREEN_PIN, LOW);
    digitalWrite(CONFIG_LED_TIC_RED_PIN, LOW);
    // Serial
    pinMode(CONFIG_TIC_DATA_PIN, INPUT);
}

/**
 * Setup function.
 * Called once MySensors has successfully initialized.
 */
void setup(void) {

    /* Setup serial port to computer */
    Serial.begin(115200);
    /* Setup tic reader */
    m_tic_reader.setup(m_tic_port);
    /* Return */
    Serial.println(" [i] Setup done.");
}

/**
 * MySensors function called to describe this sensor and its capabilites.
 */

void presentation(void) {
    /* Because messages might be lost,
     * we're not doing the presentation in one block, but rather step by step,
     * making sure each step is sucessful before advancing to the next */
    for (int8_t step = -1;;) {

        /* Send out presentation information corresponding to the current step,
         * and advance one step if successful */

        switch (step) {
            case -1: {
                if (sendSketchInfo(F("TIC Linky"), F("1.3.0")) == true) {
                    step++;
                }
                break;
            }
            case SENSOR_0_SERIAL: {
                if (present(SENSOR_0_SERIAL, S_INFO, F("TIC Serial")) == true) {  // V_TEXT ADSC
                    step++;
                }
                break;
            }
            case SENSOR_1_CURRENT: {
                if (present(SENSOR_1_CURRENT, S_MULTIMETER, F("TIC Current")) == true) {  // V_CURRENT IRMS1
                    step++;
                }
                break;
            }
            case SENSOR_2_VOLTAGE: {
                if (present(SENSOR_2_VOLTAGE, S_MULTIMETER, F("TIC Voltage")) == true) {  // V_VOLTAGE URMS1
                    step++;
                }
                break;
            }

            case SENSOR_3_POWER: {
                if (present(SENSOR_3_POWER, S_POWER, F("TIC Apparent Power")) == true) {  // V_WATT SINSTS
                    step++;
                }
                break;
            }
            case SENSOR_4_TOTAL: {
                if (present(SENSOR_4_TOTAL, S_POWER, F("TIC Energy Total")) == true) {  // V_KWH EAST
                    step++;
                }
                break;
            }
            case SENSOR_5_LAST_H: {
                if (present(SENSOR_5_LAST_H, S_POWER, F("TIC Energy last hour")) == true) {  // V_KWH CCASN
                    step++;
                }
                break;
            }
            case SENSOR_6_POWER_MAX: {
                if (present(SENSOR_6_POWER_MAX, S_POWER, F("TIC Apparent Power Max")) == true) {  // V_VA SMAXSN
                    step++;
                }
                break;
            }
            case SENSOR_7_POWER_IN: {
                if (present(SENSOR_7_POWER_IN, S_POWER, F("TIC Apparent Power Injected")) == true) {  // V_VA SINSTI
                    step++;
                }
                break;
            }
            case SENSOR_8_POWER_IN_MAX: {
                if (present(SENSOR_8_POWER_IN_MAX, S_POWER, F("TIC Apparent Power Injected Max")) == true) {  // V_VA SMAXSI
                    step++;
                }
                break;
            }
            default: {
                return;
            }
        }

        /* Sleep a little bit after each presentation, otherwise the next fails
         * @see https://forum.mysensors.org/topic/4450/sensor-presentation-failure */
        sleep(50);
    }
}


/**
 * MySensors function called when a message is received.
 */
void receive(const MyMessage &message) {
    /* For now we ignore the received message */
    (void)message;
}

/**
 * Main loop.
 */
void loop(void) {
    int res;


    /* Led task */
    /*
    {
        static uint32_t m_led_timestamp = 0;
        static enum {
            STATE_0,
            STATE_1,
            STATE_2,
            STATE_3,
            STATE_4,
            STATE_5,
            STATE_6,
            STATE_7,
            STATE_8,
            STATE_9,
        } m_led_sm;
        switch (m_led_sm) {
            case STATE_0: {
                if (m_tic_state == STATE_STARTING) {
                    m_led_sm = STATE_1;
                } else if (m_tic_state == STATE_VALID) {
                    m_led_sm = STATE_4;
                } else {
                    m_led_sm = STATE_7;
                }
                break;
            }
            case STATE_1: {
                digitalWrite(CONFIG_LED_TIC_RED_PIN, HIGH);
                digitalWrite(CONFIG_LED_TIC_GREEN_PIN, HIGH);
                m_led_timestamp = millis();
                m_led_sm = STATE_2;
                break;
            }
            case STATE_2: {
                if (millis() - m_led_timestamp >= 100) {
                    digitalWrite(CONFIG_LED_TIC_RED_PIN, LOW);
                    digitalWrite(CONFIG_LED_TIC_GREEN_PIN, LOW);
                    m_led_sm = STATE_3;
                }
                break;
            }
            case STATE_3: {
                if (millis() - m_led_timestamp >= 1000) {
                    m_led_sm = STATE_0;
                }
                break;
            }
            case STATE_4: {
                digitalWrite(CONFIG_LED_TIC_GREEN_PIN, HIGH);
                digitalWrite(CONFIG_LED_TIC_RED_PIN, LOW);
                m_led_timestamp = millis();
                m_led_sm = STATE_5;
                break;
            }
            case STATE_5: {
                if (millis() - m_led_timestamp >= 100) {
                    digitalWrite(CONFIG_LED_TIC_RED_PIN, LOW);
                    digitalWrite(CONFIG_LED_TIC_GREEN_PIN, LOW);
                    m_led_sm = STATE_6;
                }
                break;
            }
            case STATE_6: {
                if (millis() - m_led_timestamp >= 10000) {
                    m_led_sm = STATE_0;
                }
                break;
            }
            case STATE_7: {
                digitalWrite(CONFIG_LED_TIC_GREEN_PIN, LOW);
                digitalWrite(CONFIG_LED_TIC_RED_PIN, HIGH);
                m_led_timestamp = millis();
                m_led_sm = STATE_8;
                break;
            }
            case STATE_8: {
                if (millis() - m_led_timestamp >= 100) {
                    digitalWrite(CONFIG_LED_TIC_RED_PIN, LOW);
                    digitalWrite(CONFIG_LED_TIC_GREEN_PIN, LOW);
                    m_led_sm = STATE_9;
                }
                break;
            }
            case STATE_9: {
                if (millis() - m_led_timestamp >= 1000) {
                    m_led_sm = STATE_0;
                }
                break;
            }
        }
    }
    */

    /* Tic reading task */
    {
        static uint32_t refresh = millis() + PERIOD;
        uint32_t now;
        static char serial_[12 + 1];
        static uint8_t current_ = 0;
        static uint16_t voltage_  = 0;
        static uint16_t ap_ = 0;
        static uint32_t power_ = 0;
        static uint16_t powerlh_ = 0;
        static uint16_t apmax_ = 0;
        static uint16_t apin_ = 0;
        static uint16_t apinmax_ = 0;

        static enum {
            STATE_0,
            STATE_1,
        } m_tic_sm;
        switch (m_tic_sm) {

            case STATE_0: {
                m_tic_port.end();
                Serial.println("Reset Port");
                m_tic_port.begin(9600);
                m_tic_sm = STATE_1;
                break;
            }

            case STATE_1: {
                /* Read incoming datasets */
                struct tic_dataset dataset = {0};
                res = m_tic_reader.read(dataset);
                if (res < 0) {
                    Serial.println(" [e] Tic error!");
                    m_tic_state = STATE_INVALID;
                    m_tic_sm = STATE_0;
                    break;
                } else if (res == 0) {
                    break;
                }

                Serial.printf(" [d] %s = %s\r\n", dataset.name, dataset.data);
                m_tic_state = STATE_VALID;
                // force refresh every PERIOD
                // only for mostly static values
                now = millis();
                if(now > refresh) {
                  refresh = now + PERIOD;
                  serial_[0] = '\0';
                }

                /* Serial */
                if (strcmp_P(dataset.name, PSTR("ADSC")) == 0) {
                    if (strcmp(dataset.data, serial_) != 0) {
                        MyMessage message(SENSOR_0_SERIAL, V_TEXT);
                        if (send(message.set(dataset.data)) == true) {
                            strncpy(serial_, dataset.data, 12);
                        }
                    }
                }

                /* Current */
                else if (strcmp_P(dataset.name, PSTR("IRMS1")) == 0) {
                    uint8_t value = strtoul(dataset.data, NULL, 10);
                    if (value != current_) {
                        MyMessage message(SENSOR_1_CURRENT, V_CURRENT);
                        if (send(message.set(value)) == true) {
                            current_ = value;
                        }
                    }
                }

                /* Voltage */
                else if (strcmp_P(dataset.name, PSTR("URMS1")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != voltage_) {
                        MyMessage message(SENSOR_2_VOLTAGE, V_VOLTAGE);
                        if (send(message.set(value)) == true) {
                            voltage_ = value;
                        }
                    }
                }

                /* Apparent Power */
                else if (strcmp_P(dataset.name, PSTR("SINSTS")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != ap_) {
                        MyMessage message(SENSOR_3_POWER, V_VA);
                        if (send(message.set(value)) == true) {
                            ap_ = value;
                        }
                    }
                }
                /* Energy total */
                else if (strcmp_P(dataset.name, PSTR("EAST")) == 0) {
                    uint32_t value = strtoul(dataset.data, NULL, 10);
                    if (value > power_) {
                        MyMessage message(SENSOR_4_TOTAL, V_KWH);
                        if (send(message.set(value / 1000.0, 3)) == true) {
                            power_ = value;
                        }
                    }
                }
                /* Energy last hour */
                else if (strcmp_P(dataset.name, PSTR("CCASN")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value > powerlh_) {
                        MyMessage message(SENSOR_5_LAST_H, V_KWH);
                        if (send(message.set(value / 1000.0, 3)) == true) {
                            powerlh_ = value;
                        }
                    }
                }
                /* Apparent Power Max */
                else if (strcmp_P(dataset.name, PSTR("SMAXSN")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != apmax_) {
                        MyMessage message(SENSOR_6_POWER_MAX, V_VA);
                        if (send(message.set(value)) == true) {
                            apmax_ = value;
                        }
                    }
                }
                /* Apparent Power Injected */
                else if (strcmp_P(dataset.name, PSTR("SINSTI")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != apin_) {
                        MyMessage message(SENSOR_7_POWER_IN, V_VA);
                        if (send(message.set(value)) == true) {
                            apin_ = value;
                        }
                    }
                }
                /* Apparent Power Injected Max */
                else if (strcmp_P(dataset.name, PSTR("SMAXSI")) == 0) {
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != apinmax_) {
                        MyMessage message(SENSOR_8_POWER_IN_MAX, V_VA);
                        if (send(message.set(value)) == true) {
                            apinmax_ = value;
                        }
                    }
                }
                break;
            }

            default: {
                Serial.println(" [e] something got wrong");
                m_tic_sm = STATE_1;
                break;
            }
        }
    }
}
