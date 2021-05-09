/* Copyright 2021 Bastian de Byl */
#include "nxclk_handler.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "nxclk_common.h"
#include "nxclk_encoder.h"
#include "nxclk_hbdrv.h"
#include "nxclk_rtc.h"
#include "nxclk_shiftreg.h"

static volatile bool _TR_AM = true;
static volatile uint8_t _TR_H = 0x00;
static volatile uint8_t _TR_M = 0x00;

static volatile nxclk_mode _MODE = NXCLK_MODE_DISP_TIME;

nxclk_mode nxclk_get_mode(void) { return _MODE; }

void sys_tick_handler(void) {
    // Handle the function
    nxclk_handle(_MODE);
}

void nxclk_handle(nxclk_mode mode) {
    switch (mode) {
        case NXCLK_MODE_DISP_TIME:
            // Default mode, display the time via shift reg every systick hit
            nxclk_shiftout_time();
            break;
        case NXCLK_MODE_PROG_FMT:
            // Program format

            // set the shift register output to timer CNT for 24h/12h
            if (nxclk_encoder_get_bcd_value() > 1) {
                // Use AM time
                nxclk_shiftout(0x2400);
                if (!_TR_AM) _TR_AM = true;
            } else {
                // Use PM time
                nxclk_shiftout(0x1200);
                if (_TR_AM) _TR_AM = false;
            }

            break;
        case NXCLK_MODE_PROG_TIME_HR:
            // Program the time (hours)
            nxclk_shiftout((uint16_t)((nxclk_encoder_get_bcd_value() << 8) |
                                      ((_TR_M / 10) << 4) | ((_TR_M % 10))));

            break;
        case NXCLK_MODE_PROG_TIME_MIN:
            // Program the time (minutes)
            nxclk_shiftout(
                (uint16_t)((((_TR_H / 10) << 12) | (_TR_H % 10) << 8) |
                           nxclk_encoder_get_bcd_value()));

            break;
        case NXCLK_MODE_DEPOISON:
            // Cycle through all numbers every minute (use systick timer to
            // check if minute has updated) NOT IMPLEMENTED
            break;
        default:
            // Shouldn't reach here
            nxclk_set_mcu_nok();
            break;
    }
}

void nxclk_next_mode() {
    switch (_MODE) {
        case NXCLK_MODE_DISP_TIME:
            nxclk_hbdrv_disable();
            // Start the time programming; save current time (BCD?) to modify
            _TR_H = nxclk_rtc_get_hrs();
            _TR_M = nxclk_rtc_get_mins();

            nxclk_set_mode(NXCLK_MODE_PROG_FMT);
            break;
        case NXCLK_MODE_PROG_FMT:
            nxclk_set_mode(NXCLK_MODE_PROG_TIME_HR);
            break;
        case NXCLK_MODE_PROG_TIME_HR:
            _TR_H = nxclk_encoder_get();

            nxclk_set_mode(NXCLK_MODE_PROG_TIME_MIN);
            break;
        case NXCLK_MODE_PROG_TIME_MIN:
            _TR_M = nxclk_encoder_get();

            // Finished updating clock, program the final time then display
            nxclk_rtc_prog_time(_TR_H, _TR_M, _TR_AM);
            nxclk_hbdrv_enable();

            nxclk_set_mode(NXCLK_MODE_DISP_TIME);
            break;
        default:
            // Should reach here; default to displaying time
            nxclk_set_mode(NXCLK_MODE_DISP_TIME);
            break;
    }
}

void nxclk_set_mode(nxclk_mode mode) {
    // Special actions for certain modes (menu selection)
    switch (mode) {
        case NXCLK_MODE_PROG_FMT:
            nxclk_encoder_set(3, 0);
            break;
        case NXCLK_MODE_PROG_TIME_HR:
            nxclk_encoder_set(23, _TR_H);
            break;
        case NXCLK_MODE_PROG_TIME_MIN:
            nxclk_encoder_set(59, _TR_M);
            break;
        default:
            break;
    }

    _MODE = mode;
}

void nxclk_handler_start() {
    // Set up the systick timer
    systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
    STK_CVR = 0;  // Clear the counter ensuring no prior values
    systick_set_reload(rcc_ahb_frequency / 8 / 100);

    // Enable systick interrupt
    systick_interrupt_enable();

    // Enable systick timer
    systick_counter_enable();
}
