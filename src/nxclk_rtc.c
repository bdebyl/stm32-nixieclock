/* Copyright 2021 Bastian de Byl */
#include "nxclk_rtc.h"

#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>

uint8_t nxclk_rtc_get_minute(void) {
    return ((RTC_TR & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >>
            RTC_TR_MNU_SHIFT) &
           RTC_TR_MNU_MASK;
}

void nxclk_rtc_cal_init(void) {
    // Disable RTC register write protection
    rtc_unlock();

    // Enter initialization mode; calendar stopped
    rtc_set_init_flag();
    /* RTC_ISR |= RTC_ISR_INIT; */
    // Wait for confirmation of initialization mode (clock sync)
    rtc_wait_for_init_ready();

    // Program the prescaler values
    // 32.768kHz / 255+1 / 127+1 = ~1Hz
    rtc_set_prescaler(255, 127);
    rtc_set_am_format();

    // Load time and date values from the passed defines (see Makefile)
    rtc_calendar_set_date((uint8_t)TIME_YR, (uint8_t)TIME_MO, (uint8_t)TIME_DAY,
                          RTC_DR_WDU_MON);
    rtc_time_set_time((uint8_t)TIME_HR, (uint8_t)TIME_MIN, 0, true);

    // Exit initialization mode
    rtc_clear_init_flag();
    rtc_enable_bypass_shadow_register();

    // Enable the RTC register write protection
    rtc_lock();
}

void nxclk_rtc_init(void) {
    // Check if the BKP RTC register has data indicating it has already been
    // initialized
    if (NXCLK_RTC_BKPR != NXCLK_RTC_BKPR_PHRASE) {
        rcc_periph_clock_enable(RCC_PWR);

        // Send reset pulse to RCC_BDCR register
        rcc_periph_reset_pulse(RST_BDCR);

        // Disable BDCR write protection
        pwr_disable_backup_domain_write_protect();

        // Disable and wait for RCC_LSE
        rcc_osc_on(RCC_LSE);
        rcc_wait_for_osc_ready(RCC_LSE);

        // Set up the RTC clock itself
        rcc_set_rtc_clock_source(RCC_LSE);
        rcc_enable_rtc_clock();

        // Set the date and time
        nxclk_rtc_cal_init();

        NXCLK_RTC_BKPR = NXCLK_RTC_BKPR_PHRASE;
        pwr_enable_backup_domain_write_protect();
    }
}

