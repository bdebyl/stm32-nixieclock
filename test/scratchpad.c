#include <stdint.h>
#include <stdio.h>

#define RTC_TR_HT_SHIFT (20)
#define RTC_TR_HT_MASK (0x3)
#define RTC_TR_HU_SHIFT (16)
#define RTC_TR_HU_MASK (0xF)
#define RTC_TR_MNT_SHIFT (12)
#define RTC_TR_MNT_MASK (0x7)
#define RTC_TR_MNU_SHIFT (8)
#define RTC_TR_MNU_MASK (0xF)

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c %c%c%c%c"
#define BYTE_TO_BINARY(byte)                                  \
    (byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),     \
        (byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')
#define TIME_TO_DEC(byte)                                                     \
    (byte & 0x200 ? 9 : 0) + (byte & 0x100 ? 8 : 0) + (byte & 0x80 ? 7 : 0) + \
        (byte & 0x40 ? 6 : 0) + (byte & 0x20 ? 5 : 0) +                       \
        (byte & 0x10 ? 4 : 0) + (byte & 0x08 ? 3 : 0) +                       \
        (byte & 0x04 ? 2 : 0) + (byte & 0x02 ? 1 : 0) + (byte & 0x01 ? 0 : 0)
int hr_arr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23};
int main(int argc, char *argv[]) {
    /* int t; */
    uint32_t rtc_tr = 0;
    /* for (t = 0; t < 60; ++t) { */
    // Get initial time to avoid re-reading register
    uint32_t rtc_tr = 0x00121900;
    /* rtc_tr = (hr_arr[4] << RTC_TR_HU_SHIFT) |
     *          (((t / 10) % 10) << RTC_TR_MNT_SHIFT) |
     *          ((t % 10) << RTC_TR_MNU_SHIFT); */
    // 11:34

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

    /* printf("ht_shift: %i\n", ht_shift);
     * printf("hu_shift: %i\n", hu_shift);
     * printf("mnt_shift: %i\n", mnt_shift);
     * printf("mnu_shift: %i\n", mnu_shift); */
    // Shift pattern is 8-8-8-8-8, MNU must be first out, using 4 10-bit
    // wide values converted to 5 8-bit wide registers
    //
    // See: ../doc/time_shift_registers.pdf
    uint8_t shift_arr[5];
    shift_arr[0] = (1 << mnu_shift) & 0xFF;
    shift_arr[1] =
        (((1 << mnt_shift) << 2) & 0xFC) | (((1 << mnu_shift) >> 8) & 0x03);
    // Something wrong with hours
    shift_arr[2] =
        (((1 << hu_shift) << 4) & 0xF0) | (((1 << mnt_shift) >> 6) & 0x0F);
    shift_arr[3] =
        (((1 << ht_shift) << 6) & 0xC0) | (((1 << hu_shift) >> 4) & 0x3F);
    shift_arr[4] = (((1 << ht_shift) >> 2) & 0xFF);

    uint8_t mnu =
        TIME_TO_DEC(shift_arr[0]) + TIME_TO_DEC((shift_arr[1] & 0x3) << 8);
    uint8_t mnt =
        TIME_TO_DEC(shift_arr[1] >> 2) + TIME_TO_DEC((shift_arr[2] & 0xF) << 6);
    uint8_t hu = TIME_TO_DEC(shift_arr[2] >> 4) +
                 TIME_TO_DEC((shift_arr[3] & 0x3F) << 4);
    uint8_t ht =
        TIME_TO_DEC(shift_arr[3] >> 6) + TIME_TO_DEC(shift_arr[4] << 2);

    printf("t: %i, reg: %i%i:%i%i\n", t, ht, hu, mnt, mnu);
    /* uint8_t i;
     * for (i = 0; i < sizeof(shift_arr)/sizeof(shift_arr[0]); ++i) {
     *     printf("reg: %i ", i);
     *     printf("value: "BYTE_TO_BINARY_PATTERN,
     * BYTE_TO_BINARY(shift_arr[i])); printf("\n");
     * } */
    /* } */
    return 0;
}
