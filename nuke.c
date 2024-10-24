/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Obliterate the contents of flash. This is a silly thing to do if you are
// trying to run this program from flash, so you should really load and run
// directly from SRAM. You can enable RAM-only builds for all targets by doing:
//
// cmake -DPICO_NO_FLASH=1 ..
//
// in your build directory. We've also forced no-flash builds for this app in
// particular by adding:
//
// pico_set_binary_type(flash_nuke no_flash)
//
// To the CMakeLists.txt app for this file. Just to be sure, we can check the
// define:
#if !PICO_NO_FLASH && !PICO_COPY_TO_RAM
#error "This example must be built to run from SRAM!"
#endif

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

__attribute__((naked)) void isr_hardfault(void) {
    watchdog_reboot(0, 0, 0);
}

int main() {
    uint n;

    if (watchdog_caused_reboot()) {
        // Leave an eyecatcher pattern in the first page of flash so picotool can
        // more easily check the size:
        static const uint8_t eyecatcher[FLASH_PAGE_SIZE] = "NUKE";
        flash_range_program(0, eyecatcher, FLASH_PAGE_SIZE);

        reset_usb_boot(0, 0);

#ifdef PICO_DEFAULT_LED_PIN
        // Flash LED for success
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        for (int i = 0; i < 3; ++i) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(100);
        }
#endif
        return 0;
    }

    flash_range_erase(0, 2 * 1024 * 1024); // 0 - 2M

    // 2M -> 32M
    for(n = 2; n < 6; n++) {
        uint offset = (1 << (n - 1)) * 1024 * 1024;
        uint count = (1 << n) * 1024 * 1024;
        count -= offset;
        flash_range_erase(offset, count);
    }

    // Pop back up as an MSD drive
   //reset_usb_boot(0, 0);
    watchdog_reboot(0, 0, 0);
}
