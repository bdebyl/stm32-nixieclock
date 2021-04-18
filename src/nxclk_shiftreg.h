/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_SHIFTREG_H_
#define SRC_NXCLK_SHIFTREG_H_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <stdint.h>

#define NX_RCC_SPI RCC_SPI2
#define NX_RCC_GPIO_SPI RCC_GPIOB
#define NX_RCC_GPIO_REG RCC_GPIOA

#define NX_SPI SPI2
#define NX_GPIO_SPI GPIOB
#define NX_GPIO_REG GPIOA

// NX_GPIO_REG
#define NX_OE GPIO0
#define NX_RES GPIO1
#define NX_LCLK GPIO2

// NX_GPIO_SPI
#define NX_SCLK GPIO13
#define NX_MOSI GPIO15

void nxclk_shiftreg_init(void);
void nxclk_shiftout_time(void);

#endif  // SRC_NXCLK_SHIFTREG_H_

