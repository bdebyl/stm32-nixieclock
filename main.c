/* Copyright 2021 Bastian de Byl */
#include "main.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#include "src/nxclk_hbdrv.h"
#include "src/nxclk_hvctrl.h"
#include "src/nxclk_rtc.h"
#include "src/nxclk_shiftreg.h"

int main(void) {
    /* rcc_clock_setup_in_hsi_out_48mhz(); */
    /* TODO(bastian): Uncomment for actual board */
    rcc_clock_setup_in_hse_8mhz_out_48mhz();

    // MCU OK LED Setup
    rcc_periph_clock_enable(MCU_OK_RCC_GPIO);
    gpio_mode_setup(MCU_OK_GPIO_REG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    MCU_OK_GPIO_LED);
    gpio_set_output_options(MCU_OK_GPIO_REG, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,
                            MCU_OK_GPIO_LED);
    gpio_clear(MCU_OK_GPIO_REG, MCU_OK_GPIO_LED);

    // Initialize core functionalities
    nxclk_rtc_init();
    nxclk_shiftreg_init();
    nxclk_hbdrv_init();
    nxclk_hvctrl_init();

    // Set MCU OK LED to verify initialization passed
    gpio_set(MCU_OK_GPIO_REG, MCU_OK_GPIO_LED);

    // Start the desired core functionalities
    nxclk_hbdrv_enable();
    nxclk_hvctrl_enable();

    // XXX: DEBUG Shift out current time
    nxclk_shiftout_time();

    while (1)
        ;

    return 0;
}
