/*
 * Copyright (C) 2017 Obermaier Johannes
 * Copyright (C) 2022 Lucas Teske
 *
 * This Source Code Form is subject to the terms of the MIT License.
 * If a copy of the MIT License was not distributed with this file,
 * you can obtain one at https://opensource.org/licenses/MIT
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern "C" {
    #include "main.h"
    #include "reader.h"
}

static Adafruit_NeoPixel pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static void ledSet(uint8_t r, uint8_t g, uint8_t b) {
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
}

static void ledYellow() { ledSet(32, 32, 0); }
static void ledGreen()  { ledSet(0, 32, 0);  }
static void ledRed()    { ledSet(32, 0, 0);  }
static void ledBlue()   { ledSet(0, 0, 32);  }
static void ledOff()    { ledSet(0, 0, 0);   }

static void swdDiag(void) {
    swdStatus_t st;
    uint32_t id = 0;

    targetPowerOn();
    delay(POWER_ON_DELAY_MS);
    st = swdInit(&id);
    Serial.printf("diag init: %u id: %08lx\r\n", st, (unsigned long)id);
    if (st == swdStatusOk) {
        st = swdEnableDebugIF();
        Serial.printf("diag dbgIF: %u\r\n", st);
    }
    if (st == swdStatusOk) {
        st = swdSetAP32BitMode(NULL);
        Serial.printf("diag ap32: %u\r\n", st);
    }
    if (st == swdStatusOk) {
        st = swdSelectAHBAP();
        Serial.printf("diag ahb: %u\r\n", st);
    }
    targetPowerOff();
    targetReset();
    targetRestore();
}

/* One-shot SWD read with power-cycle; used to probe registers before the main loop */
static bool tryReadOnce(uint32_t const address, uint32_t* const data) {
    swdStatus_t dbgStatus = swdStatusNone;
    uint32_t idCode = 0u;

    targetPowerOn();
    delay(5);

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
        targetRestore();
        delay(1);
        dbgStatus = swdReadAHBAddr((address & 0xFFFFFFFCu), data);
    }

    targetReset();
    targetPowerOff();
    delay(2);

    return (dbgStatus == swdStatusOk);
}

void setup() {
    swdStatus_t status;
    Serial.begin(115200);

    pixel.begin();
    pixel.setBrightness(NEOPIXEL_BRIGHTNESS);
    ledOff();

    pinMode(TARGET_RESET_Pin, OUTPUT);
    pinMode(TARGET_PWR_Pin, OUTPUT);
    pinMode(SWDIO_Pin, OUTPUT);
    pinMode(SWCLK_Pin, OUTPUT);

    targetInit();
    digitalWrite(LED1_Pin, HIGH);
    ledYellow(); /* init */
    delay(150);  /* короткая пауза на инициализации */

#if NRST_DEBUG_PULSES > 0
    for (int i = 0; i < NRST_DEBUG_PULSES; ++i) {
        targetRestore();
        delay(NRST_DEBUG_HIGH_MS);
        targetReset();
        delay(NRST_DEBUG_LOW_MS);
    }
#endif

    uint32_t flashSizeBytes = FLASH_SIZE_BYTES;
    bool targetSeen = false;

    swdDiag(); /* диагностический проход, чтобы увидеть статус SWD */

#if FLASH_SIZE_AUTODETECT
    uint32_t flashSizeReg = 0u;
    if (tryReadOnce(FLASH_SIZE_REG_ADDR, &flashSizeReg)) {
        uint16_t sizeKB = (uint16_t)(flashSizeReg & 0xFFFFu);
        if (sizeKB != 0u) {
            flashSizeBytes = (uint32_t)sizeKB * 1024u;
            Serial.printf("Detected flash size: %u KB\r\n", sizeKB);
            targetSeen = true;
        } else {
            Serial.println("Flash size autodetect returned 0, fallback to default");
        }
    } else {
        Serial.println("Flash size autodetect failed, using default");
    }
#else
    Serial.printf("Flash size set statically: %lu bytes\r\n", (unsigned long)flashSizeBytes);
#endif

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
    ledBlue(); /* dumping */

    uint32_t flashData = 0;
    bool anyError = false;
    for (uint32_t i = 0; i < flashSizeBytes; i+=4) {
        flashData = 0;
        status = extractFlashData(FLASH_START_ADDR + i, &flashData);
        if (status != swdStatusOk) {
            Serial.printf("Error reading: %d\r\n", status);
            anyError = true;
            break;
        }
        targetSeen = true; /* we got at least one valid word */
        Serial.printf("%08x: %08x\r\n", FLASH_START_ADDR + i, flashData);
    }
    Serial.println("DONE");
    if (!targetSeen) {
        ledBlue(); /* нет цели на SWD */
    } else if (anyError) {
        ledRed();
    } else {
        ledGreen(); /* всё успешно */
    }
}

void loop() {

}
