/* Copyright 2021 Bastian de Byl */
#include "nxclk_handler.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include "nxclk_common.h"
#include "nxclk_shiftreg.h"

static volatile nxclk_mode _MODE = NXCLK_MODE_DISP_TIME;

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
            // NOT IMPLEMENTED

            // set the shift register output to timer CNT for 24h/12h
            break;
        case NXCLK_MODE_PROG_TIME:
            // Program the time

            // set the shift register output to timer CNT for Hours
            // set the shift register output to timer CNT for Minutes
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

void nxclk_set_mode(nxclk_mode mode) { _MODE = mode; }

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
