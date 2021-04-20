/* Copyright 2021 Bastian de Byl */
#include "nxclk_hvctrl.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

void nxclk_hvctrl_init() {
    rcc_periph_clock_enable(HVCTRL_GPIO_RCC);
    gpio_mode_setup(HVCTRL_GPIO_REG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    HVCTRL_SHDN);
    gpio_set_output_options(HVCTRL_GPIO_REG, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ,
                            HVCTRL_SHDN);
    nxclk_hvctrl_disable();
}

void nxclk_hvctrl_enable() { gpio_set(HVCTRL_GPIO_REG, HVCTRL_SHDN); }
void nxclk_hvctrl_disable() { gpio_clear(HVCTRL_GPIO_REG, HVCTRL_SHDN); }
