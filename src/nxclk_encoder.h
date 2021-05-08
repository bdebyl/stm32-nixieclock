/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_ENCODER_H_
#define SRC_NXCLK_ENCODER_H_

#include <stdint.h>

uint8_t nxclk_encoder_get_bcd_value(void);
void nxclk_encoder_set(uint32_t max_cnt, uint32_t cnt);
void nxclk_encoder_init(void);

#endif  // SRC_NXCLK_ENCODER_H_

