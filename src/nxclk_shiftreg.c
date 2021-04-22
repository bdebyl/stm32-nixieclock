/* Copyright 2021 Bastian de Byl */
#include "nxclk_shiftreg.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/spi.h>

static volatile uint8_t digit[4];
static volatile uint8_t reg_bytes[5];

// PA0: ~OE
// PA1: ~RES
// PA2: LCLK
// PB13: CLK (SPI2_SCK)
// PB15: D (SPI2_MOSI)
void nxclk_shiftreg_init() {
    rcc_periph_clock_enable(NX_RCC_SPI);
    rcc_periph_clock_enable(NX_RCC_GPIO_SPI);
    rcc_periph_clock_enable(NX_RCC_GPIO_REG);

    // Setup shift register general-purpose IO
    gpio_mode_setup(NX_GPIO_REG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                    NX_OE | NX_RES | NX_LCLK);
    gpio_set_output_options(NX_GPIO_REG, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,
                            NX_OE | NX_RES | NX_LCLK);

    // Setup shift register SPI IO
    gpio_mode_setup(NX_GPIO_SPI, GPIO_MODE_AF, GPIO_PUPD_NONE,
                    NX_SCLK | NX_MOSI);
    gpio_set_output_options(NX_GPIO_SPI, GPIO_OTYPE_PP, GPIO_OSPEED_HIGH,
                            NX_SCLK | NX_MOSI);
    gpio_set_af(NX_GPIO_SPI, GPIO_AF0, NX_SCLK | NX_MOSI);

    // Setup required SPI
    spi_set_master_mode(NX_SPI);
    spi_set_baudrate_prescaler(NX_SPI, SPI_CR1_BR_FPCLK_DIV_256);
    spi_set_clock_polarity_0(NX_SPI);
    spi_set_clock_phase_0(NX_SPI);
    spi_set_full_duplex_mode(NX_SPI);
    spi_set_unidirectional_mode(NX_SPI);
    spi_set_data_size(NX_SPI, SPI_CR2_DS_8BIT);
    // Required to allow SPI periph to send without usage of NSS
    spi_enable_software_slave_management(NX_SPI);
    spi_send_lsb_first(NX_SPI);
    spi_set_nss_high(NX_SPI);

    // Enable the NX_SPI output
    spi_enable(NX_SPI);
    // Set ~RES to high (active)
    gpio_set(NX_GPIO_REG, NX_RES);
    // Set ~OE to low (active)
    gpio_clear(NX_GPIO_REG, NX_OE);
}
void nxclk_shiftout_time() {
    // Get initial time to avoid re-reading register
    uint32_t rtc_tr = RTC_TR;

    // Take hours and minutes BCD to shifted values
    uint16_t ht_shift, hu_shift, mnt_shift, mnu_shift;
    ht_shift =
        ((rtc_tr & (RTC_TR_HT_MASK << RTC_TR_HT_SHIFT)) >> RTC_TR_HT_SHIFT) &
        0x3FF;
    hu_shift =
        ((rtc_tr & (RTC_TR_HU_MASK << RTC_TR_HU_SHIFT)) >> RTC_TR_HU_SHIFT) &
        0x3FF;
    mnt_shift =
        ((rtc_tr & (RTC_TR_MNT_MASK << RTC_TR_MNT_SHIFT)) >> RTC_TR_MNT_SHIFT) &
        0x3FF;
    mnu_shift =
        ((rtc_tr & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >> RTC_TR_MNU_SHIFT) &
        0x3FF;

    nxclk_shiftout((ht_shift << 12) | (hu_shift << 8) | (mnt_shift << 4) |
                   mnu_shift);
}
// TODO(bastian): Change this to take struct or values
void nxclk_shiftout(uint16_t digits) {
    // Shift pattern is 8-8-8-8-8, least significant number must be first out,
    // using 4 10-bit wide values converted to 5 8-bit wide registers
    //
    // See: ../doc/time_shift_registers.pdf
    digit[0] = (digits >> 12) & 0x0F;
    digit[1] = (digits >> 8) & 0x0F;
    digit[2] = (digits >> 4) & 0x0F;
    digit[3] = digits & 0x0F;

    reg_bytes[0] = (1 << (digit[3] - 2)) & 0xFF;
    reg_bytes[1] = ((1 << digit[2]) & 0xFC) | ((1 << digit[3]) & 0x03);
    reg_bytes[2] = ((1 << (digit[1] + 2)) & 0xF0) |
                   ((1 << (digit[2] + 2)) & 0x0C) |
                   ((1 << (digit[2] - 8)) & 0x03);
    reg_bytes[3] = ((1 << (digit[1] - 6)) & 0x0F) |
                   ((1 << (digit[1] + 4)) & 0x30) |
                   ((1 << (digit[0] + 4)) & 0xC0);
    reg_bytes[4] =
        ((1 << (digit[0] + 6)) & 0xC0) | ((1 << (digit[0] - 4)) & 0x3F);

    // starts in reverse with MNU, ends at HT -- first out is MNU
    gpio_clear(NX_GPIO_REG, NX_LCLK);

    uint8_t i;
    for (i = 0; i < sizeof(reg_bytes) / sizeof(reg_bytes[0]); i++) {
        spi_send8(NX_SPI, reg_bytes[i]);

        // Wait for byte to send before sending next byte
        while ((SPI_SR(NX_SPI) & SPI_SR_BSY))
            ;
    }
    // TODO(bastian): Improve this, find out why the LE is still early even if
    // SPI_SR_BSY is ongoing
    uint16_t j;
    for (j = 0; j < 0xFFFF; ++j) {
        __asm__("nop");
    }
    // end SPI transmission, latch to update outputs
    gpio_set(NX_GPIO_REG, NX_LCLK);
}

