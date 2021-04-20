/* Copyright 2021 Bastian de Byl */
#ifndef MAIN_H_
#define MAIN_H_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// TODO(bastian): Uncomment for actual board
#define MCU_OK_RCC_GPIO (RCC_GPIOA)
#define MCU_OK_GPIO_REG (GPIOA)
#define MCU_OK_GPIO_LED (GPIO15)

/* #define MCU_OK_RCC_GPIO (RCC_GPIOC)
 * #define MCU_OK_GPIO_REG (GPIOC)
 * #define MCU_OK_GPIO_LED (GPIO8) */

#endif  // MAIN_H_
