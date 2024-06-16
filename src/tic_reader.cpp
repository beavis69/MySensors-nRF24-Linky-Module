/* Self header */
#include "tic_reader.h"

#ifndef CONFIG_TIC_DEBUG_ENABLED
#define CONFIG_TIC_DEBUG_ENABLED 0 //!< Set this to 1 to turn on verbose debugging.
#endif
#if CONFIG_TIC_DEBUG_ENABLED
#ifndef CONFIG_TIC_DEBUG_FUNCTION
#define CONFIG_TIC_DEBUG_FUNCTION(x) Serial.println(F(x))
#endif
#else
#define CONFIG_TIC_DEBUG_FUNCTION(x)
#endif

// datasets we want
                    // S_INFO       (V_TEXT)    ADSC
                    // S_MULTIMETER (V_CURRENT) IRMS1
                    // S_MULTIMETER (V_CURRENT) URMS1
                    // S_POWER      (V_WATT)    SINSTS
                    // S_POWER      (V_KWH)     EAST
const uint8_t nb_wanted = 5;
const char* wanted[nb_wanted] {
  "ADSC",
  "IRMS1",
  "URMS1",
  "SINSTS",
  "EAST",
};

/**
 * @brief
 * @param[in] uart
 * @return
 */
int tic_reader::setup(Stream &uart) {

    /* Save uart */
    m_stream = &uart;

    /* Reset state machine */
    m_sm = STATE_0;

    /* Return success */
    return 0;
}

/**
 * @brief
 * @param[out] dataset
 * @return
 */
int tic_reader::read(struct tic_dataset &dataset) {
    static uint8_t splitter_count;
    static uint8_t splitter_pos[3] = {0,0,0};

    /* Ensure setup has been performed */
    if (m_stream == NULL) {
        CONFIG_TIC_DEBUG_FUNCTION(" [e] null stream!");
        return -EINVAL;
    }

    /* Process incoming bytes */
    while (m_stream->available() > 0) {

        /* Read byte */
        int rx = m_stream->read();
        if (rx < 0 || rx > UINT8_MAX) {
            CONFIG_TIC_DEBUG_FUNCTION(" [e] Read error!");
            m_sm = STATE_0;
            return -EIO;
        }
        /* Perform parity check */
        /*
        if (__builtin_parity(rx) != 0) {
            CONFIG_TIC_DEBUG_FUNCTION(" [e] Parity error!");
            m_sm = STATE_0;
            return -EIO;
        }
        */

        /* Remove parity bit */
        //rx = rx & 0x7F;
        rx &= 0x7F;
        /* Reception state machine */
        switch (m_sm) {

            case STATE_0: {  // Wait for frame start
                if (rx == 0x02) {
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] Frame start");
                    m_sm = STATE_1;
                    break;
                }
                break;
            }

            case STATE_1: {  // Wait for either dataset start or frame stop

                if (rx == 0x0A) {  // Dataset start
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] Dataset start");
                    m_dataset_buffer_index = 0;
                    m_sm = STATE_2;
                    splitter_count = 0;
                } else if (rx == 0x03) {  // Frame stop
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] Frame stop");
                    m_sm = STATE_0;
                } else {
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] waiting for new Frame");
                    m_sm = STATE_0;
                    return -EIO;
                }
                break;
            }

            case STATE_2: {  // Read dataset name
                /* If we have received one of the two possible splitter chars,
                 * take note of the type of splitter char as a way to differentiate between historic and standard datasets,
                 * move on to processing the other parts of the dataset */
                if (rx == 0x09) {
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] split char");
                    m_dataset_buffer[m_dataset_buffer_index] = '\0'; // terminate string to use strcmp
                    m_sm = STATE_SKIP;
                    for (uint8_t i = 0; i < nb_wanted; i++) {
                      // wanted ?
                      if (strcmp(m_dataset_buffer, wanted[i]) == 0) {
                        // memorize splitter count/pos
                        splitter_pos[splitter_count] = m_dataset_buffer_index;
                        splitter_count++;
                        m_dataset_buffer_index++;
                        m_sm = STATE_3;
                        break;
                      }
                    }
                    // not wanted
                    if(m_sm == STATE_SKIP) {
                      CONFIG_TIC_DEBUG_FUNCTION(" [i] skip");
                      m_dataset_buffer_index=0;
                      //splitter_count=0;
                    }
                }
                /* Otherwise, keep appending */
                else {
                    if (m_dataset_buffer_index >= TIC_PARSER_DATASET_NAME_LENGTH_MAX) {
                        CONFIG_TIC_DEBUG_FUNCTION(" [e] Dataset name too long!");
                        m_sm = STATE_0;
                        return -EIO;
                    } else {
                        m_dataset_buffer[m_dataset_buffer_index] = rx;
                        m_dataset_buffer_index++;
                    }
                }
                break;
            }

            case STATE_SKIP:
                 // skip until next dataset
                 if (rx == 0x0D) {
                   m_sm = STATE_1;
                 }
                 break;
                 
            case STATE_3: {  // Read dataset time and data

                /* If we have received the end of dataset character */
                if (rx == 0x0D) {
                    CONFIG_TIC_DEBUG_FUNCTION(" [i] End of dataset");
                    /* Verify checksum:
                     *  - for standard datasets, where the splitter char is 0x09, the checkum includes the last splitter char.
                     * Go figure */
                    uint8_t checksum_received = m_dataset_buffer[m_dataset_buffer_index - 1];
                    uint8_t checksum_computed = 0x00;
                    for (uint8_t i = 0; i < m_dataset_buffer_index - 1; i++) {
                        if(m_dataset_buffer[i] == 0) {
                          checksum_computed += 0x09;
                        } else {
                          checksum_computed += m_dataset_buffer[i];
                        }
                    }
                    checksum_computed = (checksum_computed & 0x3F) + 0x20;
                    if (checksum_computed != checksum_received) {
                        CONFIG_TIC_DEBUG_FUNCTION(" [e] Corrupt dataset!");
                        m_sm = STATE_0;
                        return -EIO;
                    }
                    
                    /* Fill dataset struct and move on */
                    if (splitter_count == 2 || splitter_count == 3) {
                        uint8_t data_pos = splitter_pos[splitter_count-2] + 1;
                        uint8_t name_length = min(TIC_PARSER_DATASET_NAME_LENGTH_MAX, splitter_pos[0]);
                        //uint8_t time_pos = splitter_pos[0] + 1;
                        //uint8_t time_length = min(TIC_PARSER_DATASET_TIME_LENGTH_MAX, splitter_pos[1] - (splitter_pos[0] + 1));
                        uint8_t data_length = min(TIC_PARSER_DATASET_DATA_LENGTH_MAX, splitter_pos[splitter_count-1] - (data_pos));
                        strncpy(dataset.name, (char *)(&m_dataset_buffer[0]), name_length);
                        //strncpy(dataset.time, (char *)(&m_dataset_buffer[time_pos]), time_length);
                        strncpy(dataset.data, (char *)(&m_dataset_buffer[data_pos]), data_length);
                        dataset.name[name_length] = '\0';
                        //dataset.time[time_length] = '\0';
                        dataset.time[0] = '\0';
                        dataset.data[data_length] = '\0';
                        CONFIG_TIC_DEBUG_FUNCTION(" [i] Dataset ok");
                        m_sm = STATE_1;
                        splitter_count=0;
                        return 1;
                    } else {
                        CONFIG_TIC_DEBUG_FUNCTION(" [e] Invalid splitter count!");
                        m_sm = STATE_0;
                        return -EIO;
                    }
                    break;
                }

                /* Otherwise, keep appending data to current dataset */
                else {
                    if (m_dataset_buffer_index >= (sizeof(m_dataset_buffer) / sizeof(char))) {
                        CONFIG_TIC_DEBUG_FUNCTION(" [e] Dataset content too long!");
                        m_sm = STATE_0;
                    } else {
                        // if its a split replace split with \0 to terminate string
                        if (rx == 0x09) {
                            CONFIG_TIC_DEBUG_FUNCTION(" [i] split char");
                            if(splitter_count <3 ) {
                              splitter_pos[splitter_count] = m_dataset_buffer_index;
                            }
                            splitter_count++;
                            m_dataset_buffer[m_dataset_buffer_index] = '\0';
                        } else {
                          m_dataset_buffer[m_dataset_buffer_index] = rx;
                        }
                        m_dataset_buffer_index++;
                    }
                }
                break;
            }

            default: {
                CONFIG_TIC_DEBUG_FUNCTION(" [e] default!");
                m_sm = STATE_0;
                break;
            }
        }
    }

    /* Return no dataset */
    return 0;
}
