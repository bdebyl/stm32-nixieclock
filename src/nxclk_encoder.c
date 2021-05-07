/* Copyright 2021 Bastian de Byl */
#include "nxclk_encoder.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "nxclk_common.h"

// ENC_SW (button): PC8
void exti4_15_isr(void) {
    if (exti_get_flag_status(EXTI8)) {
        exti_reset_request(EXTI8);

        // Toggle the button
        nxclk_toggle_mcu_ok();
    }
}

// TODO(bastian):
// - Bring out defines to header file
// - Enable counter
// - Start/stop counter to select menu mode (hold button for ~1-2 seconds)
void nxclk_encoder_init() {
    // Enable the EXTI peripheral (RCC_APB2ENR)
    rcc_periph_clock_enable(RCC_SYSCFG_COMP);
    // Set up the ENC_SW button
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO8);

    // Enable the encoder switch
    nvic_enable_irq(NVIC_EXTI4_15_IRQ);
    exti_select_source(EXTI8, GPIOC);
    exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
    exti_enable_request(EXTI8);
}

