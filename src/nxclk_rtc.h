/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_RTC_H_
#define SRC_NXCLK_RTC_H_

#include <libopencm3/stm32/rtc.h>
#include <stdint.h>

#define NXCLK_RTC_BKPR RTC_BKPXR(0)
#define NXCLK_RTC_BKPR_PHRASE (0x74696D65)

uint8_t nxclk_rtc_get_hrs(void);
uint8_t nxclk_rtc_get_mins(void);
uint8_t nxclk_rtc_get_bcd_hours(void);
uint8_t nxclk_rtc_get_bcd_minutes(void);
void nxclk_rtc_prog_time(uint8_t hours, uint8_t minutes, bool use_am_notation);
void nxclk_rtc_cal_init(void);
void nxclk_rtc_init(void);

#endif  // SRC_NXCLK_RTC_H_
