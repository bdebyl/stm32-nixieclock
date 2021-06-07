# STM32 Nixie Clock

This repository contains the source code for use with the [STM32F0 Nixie
Clock](https://github.com/bdebyl/kicad-nixieclock) KiCAD design.

## Requirements

To compile you may or may not want to edit the Makefile to your liking, but
regardless the following requirements must be met:

- `arm-none-eabi-gcc`, `-binutils`, `-newlib`
- `libopencm3` @ `0d72e673` (or newer)
- `make`
- `openocd`

## Compiling

Once the dependency requirements (_above_) are met, you must make sure your
`libopencm3` path is set correctly in the `Makefile` of this repository. By
default, this is one directory up (`../libopencm3`) -- see `OPENCM3_DIR` in
`Makefile`

Once this is satisfied, simply run `make` to compile the source! Additionally,
a helpful `flash` target is provided for use with flashing using an STLink/v2
device and OpenOCD (e.g. `make flash`)

## Disclaimer

All of this code is provided as is, with no warranties provided from or by me
in any way, shape, form, or implication whatsoever. Use and modify any of the
source code at your own risk and consult the LICENSE file for further
legalities.
