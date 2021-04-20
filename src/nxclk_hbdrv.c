/* Copyright 2021 Bastian de Byl */
#include "nxclk_hbdrv.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

volatile uint8_t _hbdrv_i = 0;
const uint16_t nxclk_hbdr_seq[] = {TIM_SR_CC1IF, TIM_SR_CC2IF, TIM_SR_CC3IF,
                                   TIM_SR_UIF};

void tim2_isr(void) {
    for (_hbdrv_i = 0;
         _hbdrv_i < sizeof(nxclk_hbdr_seq) / sizeof(nxclk_hbdr_seq[0]);
         ++_hbdrv_i) {
        if (timer_get_flag(HBDRV_TIM, nxclk_hbdr_seq[_hbdrv_i])) {
            nxclk_hbdrv_seq_isr(nxclk_hbdr_seq[_hbdrv_i]);
            timer_clear_flag(HBDRV_TIM, nxclk_hbdr_seq[_hbdrv_i]);
        }
    }
}

void nxclk_hbdrv_seq_isr(uint16_t tim_sr_ccif) {
    switch (tim_sr_ccif) {
        case TIM_SR_CC1IF:  // CH1 high CH1N low
            gpio_set(HBDRV_GPIO_REG, HBDRV_GPIO_P);
            break;
        case TIM_SR_CC2IF:  // CH1 low CH1N low
            gpio_clear(HBDRV_GPIO_REG, HBDRV_GPIO_P);
            break;
        case TIM_SR_CC3IF:  // CH1 low, CH1N high
            gpio_set(HBDRV_GPIO_REG, HBDRV_GPIO_N);
            break;
        case TIM_SR_UIF:  // CH1 low, CH1N low
            gpio_clear(HBDRV_GPIO_REG, HBDRV_GPIO_N);
            break;
        default:  // should never reach this case
            gpio_clear(HBDRV_GPIO_REG, HBDRV_GPIO_P);
            gpio_clear(HBDRV_GPIO_REG, HBDRV_GPIO_N);
            break;
    }
}

void nxclk_hbdrv_init() {
    rcc_periph_clock_enable(HBDRV_GPIO_RCC);
    rcc_periph_clock_enable(HBDRV_TIM_RCC);

    // Setup GPIO HBDRV_GPIO_P(PA7): CH1, and HBDRV_GPIO_N(PA8): CH1N
    gpio_mode_setup(HBDRV_GPIO_REG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    HBDRV_GPIO_P | HBDRV_GPIO_N);
    gpio_set_output_options(HBDRV_GPIO_REG, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ,
                            HBDRV_GPIO_P | HBDRV_GPIO_N);

    // Setup the H-bridge driver timer
    timer_set_mode(HBDRV_TIM, TIM_CR1_CKD_CK_INT_MUL_4, TIM_CR1_CMS_EDGE,
                   TIM_CR1_DIR_UP);
    timer_continuous_mode(HBDRV_TIM);

    // Period (60Hz)
    timer_set_prescaler(HBDRV_TIM, HBDRV_TIM_PSC);
    timer_set_period(HBDRV_TIM, HBDRV_TIM_PER);

    // Setup half-bridge driver waveform (pseudo-sine)
    timer_set_oc_mode(HBDRV_TIM, TIM_OC1, TIM_OCM_PWM2);
    timer_set_oc_value(HBDRV_TIM, TIM_OC1, HBDRV_TIM_CC1);
    timer_set_oc_mode(HBDRV_TIM, TIM_OC2, TIM_OCM_PWM2);
    timer_set_oc_value(HBDRV_TIM, TIM_OC2, HBDRV_TIM_CC2);
    timer_set_oc_mode(HBDRV_TIM, TIM_OC3, TIM_OCM_PWM2);
    timer_set_oc_value(HBDRV_TIM, TIM_OC3, HBDRV_TIM_CC3);
    // Resets on Update event instead of using CC4 at near-max value of TIM_CNT

    nvic_set_priority(HBDRV_NVIC_IRQ, 1);
}

void nxclk_hbdrv_enable() {
    timer_enable_irq(HBDRV_TIM, TIM_DIER_CC1IE | TIM_DIER_CC2IE |
                                    TIM_DIER_CC3IE | TIM_DIER_UIE);
    timer_update_on_any(HBDRV_TIM);

    nvic_enable_irq(HBDRV_NVIC_IRQ);
    timer_enable_counter(HBDRV_TIM);
}

void nxclk_hbdrv_disable() {
    timer_disable_irq(HBDRV_TIM, TIM_DIER_CC1IE | TIM_DIER_CC2IE |
                                     TIM_DIER_CC3IE | TIM_DIER_UIE);
    timer_disable_update_event(HBDRV_TIM);

    nvic_disable_irq(HBDRV_NVIC_IRQ);
    timer_disable_counter(HBDRV_TIM);
}
