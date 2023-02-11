ifndef WONDERFUL_TOOLCHAIN
$(error Please define WONDERFUL_TOOLCHAIN to point to the location of the Wonderful toolchain.)
endif
include $(WONDERFUL_TOOLCHAIN)/i8086/wswan.mk

TARGET ?= generic

OBJDIR := obj/$(TARGET)
MKDIRS := $(OBJDIR)
LIBS := -lws -lc -lgcc
CFLAGS += -Os -fno-jump-tables -ffunction-sections
SLFLAGS := --heap-length 0x1800 --rtc --color --rom-size 2 --ram-type SRAM_512KB --unlock-ieep
LDFLAGS += -Wl,--gc-sections

SRCDIRS := res src src/$(TARGET)
CSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.S)))
OBJECTS := $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.S=$(OBJDIR)/%.o)

CFLAGS += -Ires -Isrc -DTARGET_$(TARGET)

DEPS := $(OBJECTS:.o=.d)
CFLAGS += -MMD -MP

vpath %.c $(SRCDIRS)
vpath %.S $(SRCDIRS)

.PHONY: all clean install

all: CartFriend_$(TARGET).wsc

CartFriend_$(TARGET).wsc: $(OBJECTS) | $(OJBDIR)
	$(SWANLINK) -v -o $@ --publisher-id 170 --game-id 55 --output-elf $@.elf $(SLFLAGS) --linker-args $(LDFLAGS) $(OBJECTS) $(OBJECTS_FLASH_MASTA) $(LIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(info $(shell mkdir -p $(MKDIRS)))

clean:
	rm -r $(OBJDIR)/*
	rm CartFriend_$(TARGET).wsc CartFriend_$(TARGET).elf

-include $(DEPS)
