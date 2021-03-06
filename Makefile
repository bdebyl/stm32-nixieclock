OBJS = main.o $(patsubst %.c,%.o,$(wildcard src/*.c))

OPENCM3_DIR := ../libopencm3
#/usr/arm-none-eabi

# Our output name
BINARY = main
DISCO ?= 0

LDSCRIPT = stm32f0.ld

# Current time for RTC programming
# TODO: Improve this to use cut or substring?
TIME_HR      = $(shell date +'%H' | sed 's/^0//g')
TIME_MIN     = $(shell date +'%M' | sed 's/^0//g')
TIME_YR      = $(shell date +'%y' | sed 's/^0//g')
TIME_MO      = $(shell date +'%m' | sed 's/^0//g')
TIME_DAY     = $(shell date +'%d' | sed 's/^0//g')

TIME_DEFS    = -DTIME_HR=${TIME_HR} -DTIME_MIN=${TIME_MIN} -DTIME_YR=${TIME_YR} -DTIME_MO=${TIME_MO} -DTIME_DAY=${TIME_DAY}

# Using the stm32f0 series chip
LIBNAME      = opencm3_stm32f0
DEFS        += -DSTM32F0

# Target-specific flags
FP_FLAGS    ?= -msoft-float
ARCH_FLAGS   = -mthumb -mcpu=cortex-m0 $(FP_FLAGS)

OOCD         = $(shell command -v openocd)
OOCD_IF      = interface/stlink-v2.cfg
OOCD_TG      = target/stm32f0x.cfg
STFLASH      = $(shell command -v st-flash)

# Compiler configuration
PREFIX      ?= arm-none-eabi
CC          := $(PREFIX)-gcc
LD          := $(PREFIX)-gcc
AR          := $(PREFIX)-ar
AS          := $(PREFIX)-as
SIZE        := $(PREFIX)-size
OBJCOPY     := $(PREFIX)-objcopy
OBJDUMP     := $(PREFIX)-objdump
GDB         := $(PREFIX)-gdb
OPT         := -Os
DEBUG       := -ggdb3
CSTD        ?= -std=c99

# C flags
CFLAGS       = $(TIME_DEFS)
ifeq ($(DISCO), 1)
CFLAGS      += -DDISCO
endif
TGT_CFLAGS  += $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS  += $(ARCH_FLAGS)
TGT_CFLAGS  += -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS  += -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS  += -fno-common -ffunction-sections -fdata-sections

# C preprocessor common flags
TGT_CFLAGS    += -MD
TGT_CFLAGS    += -Wall -Wundef
TGT_CFLAGS    += $(DEFS)

# Linker flags
TGT_LDFLAGS     += --static -nostartfiles
TGT_LDFLAGS     += -T$(LDSCRIPT)
TGT_LDFLAGS     += $(ARCH_FLAGS) $(DEBUG)
TGT_LDFLAGS     += -Wl,-Map=$(*).map -Wl,--cref
TGT_LDFLAGS     += -Wl,--gc-sections

# Used libraries
DEFS        += -I$(OPENCM3_DIR)/include
LDFLAGS     += -L$(OPENCM3_DIR)/lib
LDLIBS      += -l$(LIBNAME)
LDLIBS      += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

all: size

$(OPENCM3_DIR)/lib/lib$(LIBNAME).a:
ifeq (,$(wildcard $@))
	$(warning $(LIBNAME).a not found, attempting to rebuild in $(OPENCM3_DIR))
	$(MAKE) -C $(OPENCM3_DIR)
endif

size: $(BINARY).size
elf: $(BINARY).elf
bin: $(BINARY).bin
map: $(BINARY).map

GENERATED_BINARIES=$(BINARY).elf $(BINARY).bin $(BINARY).map

%.bin: %.elf
	$(OBJCOPY) -Obinary $(*).elf $(*).bin

%.elf %.map: $(OBJS) $(LDSCRIPT)
	$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c %.h $(OPENCM3_DIR)/lib/lib$(LIBNAME).a
	$(CC) $(TGT_CFLAGS) $(CFLAGS)  -o $(*).o -c $(*).c

%.size: %.elf
	@$(SIZE) -d $(*).elf | tee $(*).size

flash: $(BINARY).elf
	@printf "  FLASH  $<\n"
	$(OOCD) -f $(OOCD_IF) -f $(OOCD_TG) -c "program $(BINARY).elf verify reset exit"

# flash-stlink: $(BINARY).elf
# 	@printf "  FLASH  $<\n"
# 	$(STFLASH) write $(BINARY).bin 0x8000000

.PHONY: reset
reset:
	$(STFLASH)

.PHONY: clean
clean:
	$(RM) $(GENERATED_BINARIES) generated.* $(OBJS) $(OBJS:%.o=%.d)

.PHONY: lint
lint:
	cpplint main.{c,h} src/*.{c,h}
