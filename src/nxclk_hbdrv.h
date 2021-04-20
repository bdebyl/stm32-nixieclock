/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_HBDRV_H_
#define SRC_NXCLK_HBDRV_H_

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#define HBDRV_GPIO_RCC (RCC_GPIOA)
#define HBDRV_TIM_RCC (RCC_TIM2)
#define HBDRV_NVIC_IRQ (NVIC_TIM2_IRQ)

#define HBDRV_TIM (TIM2)
#define HBDRV_GPIO_REG (GPIOA)

#define HBDRV_GPIO_P (GPIO7)
#define HBDRV_GPIO_N (GPIO8)

// H-Bridge driver timing:
// Creating a full 60Hz sinze wave with full-wave off time between pulses
#define HBDRV_TIM_PSC (15)
#define HBDRV_TIM_PER (50000)
#define HBDRV_TIM_PER_DIV (HBDRV_TIM_PER / 4)
#define HBDRV_TIM_CC1 (HBDRV_TIM_PER_DIV - 1)
#define HBDRV_TIM_CC2 (HBDRV_TIM_PER_DIV * 2 - 1)
#define HBDRV_TIM_CC3 (HBDRV_TIM_PER_DIV * 3 - 1)

void nxclk_hbdrv_seq_isr(uint16_t tim_sr_ccif);
void nxclk_hbdrv_init(void);
void nxclk_hbdrv_enable(void);
void nxclk_hbdrv_disable(void);

#endif  // SRC_NXCLK_HBDRV_H_

