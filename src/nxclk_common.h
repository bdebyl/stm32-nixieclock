/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_COMMON_H_
#define SRC_NXCLK_COMMON_H_

typedef enum {
  NXCLK_MODE_DISP_TIME,
  NXCLK_MODE_PROG_FMT,
  NXCLK_MODE_PROG_TIME_HR,
  NXCLK_MODE_PROG_TIME_MIN,
  NXCLK_MODE_DEPOISON
} nxclk_mode;

#ifndef DISCO
#define MCU_OK_RCC_GPIO (RCC_GPIOA)
#define MCU_OK_GPIO_REG (GPIOA)
#define MCU_OK_GPIO_LED (GPIO15)
#else
#define MCU_OK_RCC_GPIO (RCC_GPIOC)
#define MCU_OK_GPIO_REG (GPIOC)
#define MCU_OK_GPIO_LED (GPIO8)
#endif

void nxclk_common_init(void);
void nxclk_set_mcu_ok(void);
void nxclk_set_mcu_nok(void);
void nxclk_toggle_mcu_ok(void);

#endif  // SRC_NXCLK_COMMON_H_
