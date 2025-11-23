/*
 * Copyright (C) 2017 Obermaier Johannes
 * Copyright (C) 2022 Lucas Teske
 *
 * This Source Code Form is subject to the terms of the MIT License.
 * If a copy of the MIT License was not distributed with this file,
 * you can obtain one at https://opensource.org/licenses/MIT
 */

#pragma once

#include <stdint.h>

#include "Arduino.h"

#define BUTTON1_Pin 4
#define LED1_Pin 25
#define TARGET_RESET_Pin 17   /* перенесли reset на GP17 (pad 19) */
#define TARGET_PWR_Pin 26
#define SWDIO_Pin 14
#define SWCLK_Pin 15

/* Debug: generate visible NRST pulses at startup to verify wiring */
#define NRST_DEBUG_PULSES 3
#define NRST_DEBUG_HIGH_MS 40
#define NRST_DEBUG_LOW_MS 40

/* On-board addressable LED (RP2040-Tiny / WS2812) */
#define NEOPIXEL_PIN        16
#define NEOPIXEL_COUNT      1
#define NEOPIXEL_BRIGHTNESS 40  /* 0-255 */

#define MAX_READ_ATTEMPTS (100u)

/* Flash layout (change to match your target) */
#define FLASH_SIZE_BYTES (256u * 1024u)   /* fallback: STM32F091CC 256 KB */
#define FLASH_START_ADDR (0x08000000u)    /* start of flash */

/* Try to auto-detect flash size once before the attack (STM32F0: 0x1FFFF7CC holds size in KB). */
#define FLASH_SIZE_AUTODETECT (1u)
#define FLASH_SIZE_REG_ADDR   (0x1FFFF7CCu)

/* Auto-start timeout for the dump after power-up.
 * 0 -> wait forever for a key on the serial port.
 * Any other value -> milliseconds to wait, then start automatically. */
#define START_TIMEOUT_MS (5000u)

/* all times in milliseconds */
/* minimum wait time between reset deassert and attack */
/* With my test devices this works as 0. Obermaier's default is 20 */
// #define DELAY_JITTER_MS_MIN (20u)
/* 2 ms минимум, чтобы на NRST был видимый импульс и чип успел проснуться */
#define DELAY_JITTER_MS_MIN (2u)
/* increment per failed attack */
#define DELAY_JITTER_MS_INCREMENT (1u)
/* maximum wait time between reset deassert and attack */
#define DELAY_JITTER_MS_MAX (50u)

void targetInit(void);

void targetReset(void);

void targetRestore(void);

void targetPowerOff(void);

void targetPowerOn(void);
