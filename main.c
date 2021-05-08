/* Copyright 2021 Bastian de Byl */
#include "main.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#include "src/nxclk_common.h"
#include "src/nxclk_encoder.h"
#include "src/nxclk_hbdrv.h"
#include "src/nxclk_hvctrl.h"
#include "src/nxclk_init_tubes.h"
#include "src/nxclk_rtc.h"
#include "src/nxclk_shiftreg.h"

int main(void) {
    // Set up the system clock depending on usage of DISCOvery board (-DDISCO)
#ifndef DISCO
    rcc_clock_setup_in_hse_8mhz_out_48mhz();
#else
    rcc_clock_setup_in_hsi_out_48mhz();
#endif
    nxclk_common_init();

    // Peripheral initialization
    nxclk_encoder_init();
    nxclk_tubes_init();
    nxclk_shiftreg_init();
    nxclk_hbdrv_init();
    nxclk_hvctrl_init();

    // Start the desired core functionalities
    nxclk_hbdrv_enable();
    nxclk_hvctrl_enable();

    // Start the show!
    nxclk_tubes_cycle_and_start();

    while (1)
        ;

    return 0;
}
