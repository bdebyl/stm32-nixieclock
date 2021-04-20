/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_HVCTRL_H_
#define SRC_NXCLK_HVCTRL_H_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

// ~SHDN (PA3; active-high with pull-down resistor)
#define HVCTRL_GPIO_RCC (RCC_GPIOA)
#define HVCTRL_GPIO_REG (GPIOA)
#define HVCTRL_SHDN (GPIO3)

void nxclk_hvctrl_init(void);
void nxclk_hvctrl_enable(void);
void nxclk_hvctrl_disable(void);

#endif  // SRC_NXCLK_HVCTRL_H_

