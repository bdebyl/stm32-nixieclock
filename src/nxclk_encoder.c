/* Copyright 2021 Bastian de Byl */
#include "nxclk_encoder.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "nxclk_common.h"
#include "nxclk_handler.h"
#include "nxclk_hbdrv.h"

// ENC_A | ENC_B (knob)
void tim3_isr(void) {
    if (timer_get_flag(TIM3, TIM_SR_UIF)) {
        // TODO(bastian): figure out logic here for display value
        timer_clear_flag(TIM3, TIM_SR_UIF);
    }
}

// ENC_SW (button): PC8
void exti4_15_isr(void) {
    if (exti_get_flag_status(EXTI8)) {
        exti_reset_request(EXTI8);

        // Toggle the button
        /* nxclk_toggle_mcu_ok(); */

        // Toggle the mode
        if (nxclk_get_mode() == NXCLK_MODE_DISP_TIME) {
            nxclk_hbdrv_disable();
            nxclk_set_mode(NXCLK_MODE_PROG_TIME_HR);
        } else {
            nxclk_hbdrv_enable();
            nxclk_set_mode(NXCLK_MODE_DISP_TIME);
        }
    }
}

uint8_t nxclk_encoder_get_bcd_value() {
    uint32_t count = timer_get_counter(TIM3);
    return (((count / 10) << 4) | (count % 10)) & 0xFF;
}

void nxclk_encoder_set_max(uint32_t max_cnt) {
    timer_set_counter(TIM3, 1);
    timer_set_period(TIM3, max_cnt);
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

    // Encoder knob on TIM3_CH1 (PC6) and TIM3_CH2 (PC7)
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
                            GPIO6 | GPIO7);

    gpio_set_af(GPIOC, GPIO_AF2, GPIO6 | GPIO7);

    // TI1 connected to TI1FP1 CC1S='01' in TIMx_CCMR1
    timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
    // TI2 connected to TI2FP2 CC2s='01' in TIMx_CCMR2
    timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);
    // CC1P='0' and CC1NP='0'(CCER register, T1FP1 non-inverted, TI1FP1=TI1)
    timer_ic_set_polarity(TIM3, TIM_IC1, TIM_IC_RISING);
    // CC2P='0' and CC2NP='0'(CCER register, T1FP2 non-inverted, TI1FP2=TI1)
    timer_ic_set_polarity(TIM3, TIM_IC2, TIM_IC_RISING);

    // Note:
    /* The STM32 has 3 different encoder modes, details of this are found in
     * AN4013. The details of this are found in AN4013. Ultimately the two
     * modes are X2 and X4 resolution. This means the TIMES 2 or TIMES 4. For
     * example if the encoder knob detents once on turning it will count
     * (up/down) by TWO in X2 and by FOUR in X4. This leaves the one bit mostly
     * unseen/unused between detents. To use this for fine counting either
     * shift the TIMx_CNT value over once or divide-by 2 in X2 or 4 in X4 (same
     * operation).
     * */

    // Select Encoder interface mode wiht SMS='001'
    timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM1);

    /* timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
     * TIM_CR1_DIR_UP); */
    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    /* timer_set_prescaler(TIM3, 0); */

    timer_ic_set_filter(TIM3, TIM_IC1, TIM_IC_CK_INT_N_2);
    timer_ic_set_prescaler(TIM3, TIM_IC1, 0);

    timer_ic_set_filter(TIM3, TIM_IC2, TIM_IC_CK_INT_N_2);
    timer_ic_set_prescaler(TIM3, TIM_IC2, 0);

    timer_set_counter(TIM3, 0);

    // TODO(bastian): change the period depending on _MODE
    timer_set_period(TIM3, 1);
    timer_update_on_any(TIM3);
    timer_enable_irq(TIM3, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM3_IRQ);

    timer_ic_enable(TIM3, TIM_IC1);
    timer_ic_enable(TIM3, TIM_IC2);

    timer_continuous_mode(TIM3);
    timer_enable_counter(TIM3);
}

