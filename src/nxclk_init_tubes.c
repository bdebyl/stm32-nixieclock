/* Copyright 2021 Bastian de Byl */
#include "nxclk_init_tubes.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "nxclk_handler.h"
#include "nxclk_shiftreg.h"

static volatile uint8_t nx_test_cnt = 0;
static volatile uint8_t minute = 0;

void tim14_isr(void) {
    if (TIM_SR(TIM14) & TIM_SR_UIF) {
        // Test next nixie
        nxclk_shiftout((nx_test_cnt << 12) | (nx_test_cnt << 8) |
                       (nx_test_cnt << 4) | nx_test_cnt);
        nx_test_cnt++;

        if (nx_test_cnt > 10) {
            timer_disable_counter(TIM14);

            // Digit cycling complete, enable handler (systick)
            nxclk_handler_start();
        }

        timer_clear_flag(TIM14, TIM_SR_UIF);
    }
}

void nxclk_tubes_init() {
    // Initial clock test
    rcc_periph_clock_enable(RCC_TIM14);
    timer_set_prescaler(TIM14, 48000);
    timer_set_period(TIM14, 600);
    timer_enable_update_event(TIM14);
    timer_update_on_any(TIM14);
    timer_enable_irq(TIM14, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM14_IRQ);
    timer_continuous_mode(TIM14);
}

void nxclk_tubes_cycle_and_start() { timer_enable_counter(TIM14); }
