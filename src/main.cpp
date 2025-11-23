/*
 * Copyright (C) 2017 Obermaier Johannes
 * Copyright (C) 2022 Lucas Teske
 *
 * This Source Code Form is subject to the terms of the MIT License.
 * If a copy of the MIT License was not distributed with this file,
 * you can obtain one at https://opensource.org/licenses/MIT
 */

#include <Arduino.h>

extern "C" {
    #include "main.h"
    #include "reader.h"
}

void setup() {
    swdStatus_t status;
    Serial.begin(115200);

    pinMode(TARGET_RESET_Pin, OUTPUT);
    pinMode(TARGET_PWR_Pin, OUTPUT);
    pinMode(SWDIO_Pin, OUTPUT);
    pinMode(SWCLK_Pin, OUTPUT);

    targetInit();
    digitalWrite(LED1_Pin, HIGH);

    unsigned long startWait = millis();
    while (!Serial.available()) {
        if ((START_TIMEOUT_MS > 0u) && (millis() - startWait >= START_TIMEOUT_MS)) {
            Serial.println("No input, auto-starting...");
            break;
        }
        delay(1000);
        Serial.println("Send anything to start...");
    }
    // flush the byte if user sent something
    if (Serial.available()) {
        Serial.read();
    }
    Serial.println("Starting");

    uint32_t flashData = 0;
    for (uint32_t i = 0; i < FLASH_SIZE_BYTES; i+=4) {
        flashData = 0;
        status = extractFlashData(FLASH_START_ADDR + i, &flashData);
        if (status != swdStatusOk) {
            Serial.printf("Error reading: %d\r\n", status);
            break;
        }
        Serial.printf("%08x: %08x\r\n", FLASH_START_ADDR + i, flashData);
    }
    Serial.println("DONE");
}

void loop() {

}
