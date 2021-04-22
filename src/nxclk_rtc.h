/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_RTC_H_
#define SRC_NXCLK_RTC_H_

#include <libopencm3/stm32/rtc.h>

#define NXCLK_RTC_BKPR RTC_BKPXR(0)
#define NXCLK_RTC_BKPR_PHRASE (0x74696D65)

void nxclk_rtc_cal_init(void);
void nxclk_rtc_init(void);

#endif  // SRC_NXCLK_RTC_H_
