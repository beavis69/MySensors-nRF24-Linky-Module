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

/* Working variables */
static SoftwareSerial m_tic_port(CONFIG_TIC_DATA_PIN, CONFIG_TIC_DUMMY_PIN);
static tic_reader m_tic_reader;
static enum {
    STATE_STARTING,
    STATE_VALID,
    STATE_INVALID,
} m_tic_state;

/* List of virtual sensors */
enum {
    SENSOR_0_SERIAL,         // S_INFO       (V_TEXT)    ADSC
    SENSOR_1_CURRENT,        // S_MULTIMETER (V_CURRENT) IRMS1
    SENSOR_2_VOLTAGE,        // S_MULTIMETER (V_CURRENT) URMS1
    SENSOR_3_POWER,          // S_POWER      (V_WATT)    SINSTS
    SENSOR_4_TOTAL,          // S_POWER      (V_KWH)     EAST
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
        static enum {
            STATE_0,
            STATE_1,
        } m_tic_sm;
        switch (m_tic_sm) {

            case STATE_0: {
                m_tic_port.end();
                pinMode(CONFIG_TIC_DATA_PIN, INPUT);
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

                Serial.printf(" [d] Received dataset %s = %s\r\n", dataset.name, dataset.data);
                m_tic_state = STATE_VALID;

                /* Numéro de Série */
                if (strcmp_P(dataset.name, PSTR("ADSC")) == 0) {
                    static char serial_[12 + 1];
                    if (strcmp(dataset.data, serial_) != 0) {
                        MyMessage message(SENSOR_0_SERIAL, V_TEXT);
                        if (send(message.set(dataset.data)) == true) {
                            strncpy(serial_, dataset.data, 12);
                        }
                    }
                }

                /* Intensité Phase 1 */
                else if (strcmp_P(dataset.name, PSTR("IRMS1")) == 0) {
                    static uint8_t current_ = 0;
                    uint8_t value = strtoul(dataset.data, NULL, 10);
                    if (value != current_) {
                        MyMessage message(SENSOR_1_CURRENT, V_CURRENT);
                        if (send(message.set(value)) == true) {
                            current_ = value;
                        }
                    }
                }

                /* Tension Phase 1 */
                else if (strcmp_P(dataset.name, PSTR("URMS1")) == 0) {
                    static uint16_t voltage_  = 0;
                    uint16_t value = strtoul(dataset.data, NULL, 10);
                    if (value != voltage_) {
                        MyMessage message(SENSOR_2_VOLTAGE, V_VOLTAGE);
                        if (send(message.set(value)) == true) {
                            voltage_ = value;
                        }
                    }
                }

                /* Puissance apparente */
                else if (strcmp_P(dataset.name, PSTR("SINSTS")) == 0) {
                    static uint32_t pa_ = 0;
                    uint32_t value = strtoul(dataset.data, NULL, 10);
                    if (value != pa_) {
                        MyMessage message(SENSOR_3_POWER, V_VA);
                        if (send(message.set(value)) == true) {
                            pa_ = value;
                        }
                    }
                }
                /* Option Base, index TH */
                else if (strcmp_P(dataset.name, PSTR("EAST")) == 0) {
                    static uint32_t power_ = 0;
                    uint32_t value = strtoul(dataset.data, NULL, 10);
                    if (value > power_) {
                        MyMessage message(SENSOR_4_TOTAL, V_KWH);
                        if (send(message.set(value / 1000.0, 3)) == true) {
                            power_ = value;
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
