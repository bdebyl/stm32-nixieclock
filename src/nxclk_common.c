/* Copyright 2021 Bastian de Byl */
#include "nxclk_common.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

void nxclk_common_init() {
    rcc_periph_clock_enable(MCU_OK_RCC_GPIO);
    gpio_mode_setup(MCU_OK_GPIO_REG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    MCU_OK_GPIO_LED);
    gpio_set_output_options(MCU_OK_GPIO_REG, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,
                            MCU_OK_GPIO_LED);
}

void nxclk_set_mcu_ok() { gpio_set(MCU_OK_GPIO_REG, MCU_OK_GPIO_LED); }
void nxclk_set_mcu_nok() { gpio_clear(MCU_OK_GPIO_REG, MCU_OK_GPIO_LED); }
void nxclk_toggle_mcu_ok() { gpio_toggle(MCU_OK_GPIO_REG, MCU_OK_GPIO_LED); }
