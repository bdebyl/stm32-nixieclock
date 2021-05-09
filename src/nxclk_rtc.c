/* Copyright 2021 Bastian de Byl */
#include "nxclk_rtc.h"

#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>

/* BEGIN STATIC DEFS */
static uint8_t _nxclk_rtc_get_secs(void) {
    return ((((RTC_TR & (RTC_TR_ST_MASK << RTC_TR_ST_SHIFT)) >>
              RTC_TR_ST_SHIFT) &
             RTC_TR_ST_MASK) *
            10) +
           (((RTC_TR & (RTC_TR_SU_MASK << RTC_TR_SU_SHIFT)) >>
             RTC_TR_SU_SHIFT) &
            RTC_TR_SU_MASK);
}

// TODO(bastian): test this to make sure it works!
static void _nxclk_rtc_update_fmt(void) {
    uint8_t h = nxclk_rtc_get_hrs();

    // Check what the notation is
    if (RTC_CR & RTC_CR_FMT) {
        // Convert 24h to 12h am/pm
        if (h > 12) {
            // Get new hour value
            h = h - 12;

            // Set the new value and the PM bit
            rtc_time_set_time(h, nxclk_rtc_get_mins(), _nxclk_rtc_get_secs(),
                              false);
        }
    } else {
        // Convert 12h am/pm to 24h
        if (RTC_TR & RTC_TR_PM) {
            h = h + 12;

            // Set the new value and clear the PM bit
            rtc_time_set_time(h, nxclk_rtc_get_mins(), _nxclk_rtc_get_secs(),
                              true);
        }
    }
}
/* END STATIC DEFS */

uint8_t nxclk_rtc_get_hrs() {
    return ((((RTC_TR & (RTC_TR_HT_MASK << RTC_TR_HT_SHIFT)) >>
              RTC_TR_HT_SHIFT) &
             RTC_TR_HT_MASK) *
            10) +
           (((RTC_TR & (RTC_TR_HU_MASK << RTC_TR_HU_SHIFT)) >>
             RTC_TR_HU_SHIFT) &
            RTC_TR_HU_MASK);
}

uint8_t nxclk_rtc_get_mins() {
    return ((((RTC_TR & (RTC_TR_MNT_MASK << RTC_TR_MNT_SHIFT)) >>
              RTC_TR_MNT_SHIFT) &
             RTC_TR_MNT_MASK) *
            10) +
           (((RTC_TR & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >>
             RTC_TR_MNU_SHIFT) &
            RTC_TR_MNU_MASK);
}

uint8_t nxclk_rtc_get_bcd_hours() {
    return ((((RTC_TR & (RTC_TR_HT_MASK << RTC_TR_HT_SHIFT)) >> RTC_TR_HT_SHIFT)
             << 4) &
            0xF0) |
           (((RTC_TR & (RTC_TR_HU_MASK << RTC_TR_HU_SHIFT)) >>
             RTC_TR_HU_SHIFT) &
            0x0F);
}

uint8_t nxclk_rtc_get_bcd_minutes() {
    return ((((RTC_TR & (RTC_TR_MNT_MASK << RTC_TR_MNT_SHIFT)) >>
              RTC_TR_MNT_SHIFT)
             << 4) &
            0xF0) |
           (((RTC_TR & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >>
             RTC_TR_MNU_SHIFT) &
            0x0F);
}

void nxclk_rtc_prog_time(uint8_t hours, uint8_t minutes, bool use_am_notation) {
    rcc_periph_clock_enable(RCC_PWR);

    // Disable BDCR write protection
    pwr_disable_backup_domain_write_protect();

    // Disable RTC register write protection
    rtc_unlock();

    // Enter initialization mode; calendar stopped
    rtc_set_init_flag();
    /* RTC_ISR |= RTC_ISR_INIT; */
    // Wait for confirmation of initialization mode (clock sync)
    rtc_wait_for_init_ready();

    if (use_am_notation) {
        rtc_set_am_format();
    } else {
        rtc_set_pm_format();
    }

    rtc_time_set_time(hours, minutes, 0, use_am_notation);

    // Update 24h/12h display format
    _nxclk_rtc_update_fmt();

    // Exit initialization mode
    rtc_clear_init_flag();
    rtc_enable_bypass_shadow_register();

    // Enable the RTC register write protection
    rtc_lock();

    // Disable BDCR write protection
    pwr_enable_backup_domain_write_protect();
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

    // TODO(bastian): Break this out to allow re-programming
    rtc_time_set_time((uint8_t)TIME_HR, (uint8_t)TIME_MIN, 0, true);

    // Update 24h/12h display format
    _nxclk_rtc_update_fmt();

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
#ifndef DISCO
        rcc_osc_on(RCC_LSE);
        rcc_wait_for_osc_ready(RCC_LSE);
        rcc_set_rtc_clock_source(RCC_LSE);
#else
        rcc_osc_on(RCC_LSI);
        rcc_wait_for_osc_ready(RCC_LSI);
        rcc_set_rtc_clock_source(RCC_LSI);
#endif
        rcc_enable_rtc_clock();

        // Set the date and time
        nxclk_rtc_cal_init();

        NXCLK_RTC_BKPR = NXCLK_RTC_BKPR_PHRASE;
        pwr_enable_backup_domain_write_protect();
    }
}

