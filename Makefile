ifndef WONDERFUL_TOOLCHAIN
$(error Please define WONDERFUL_TOOLCHAIN to point to the location of the Wonderful toolchain.)
endif
include $(WONDERFUL_TOOLCHAIN)/i8086/wswan.mk

OBJDIR := obj
MKDIRS := $(OBJDIR)
LIBS := -lws -lc -lgcc
CFLAGS += -Os -fno-jump-tables
SLFLAGS := --heap-length 0x1800 --rom-size 2 --ram-type SRAM_8KB --unlock-ieep

SRCDIRS := res src
CSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.S)))
OBJECTS := $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.S=$(OBJDIR)/%.o)

SRCDIRS_STUB := src/stub
CSOURCES_STUB := $(foreach dir,$(SRCDIRS_STUB),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES_STUB := $(foreach dir,$(SRCDIRS_STUB),$(notdir $(wildcard $(dir)/*.S)))
OBJECTS_STUB := $(CSOURCES_STUB:%.c=$(OBJDIR)/%.o) $(ASMSOURCES_STUB:%.S=$(OBJDIR)/%.o)

SRCDIRS_FLASH_MASTA := src/flash_masta
CSOURCES_FLASH_MASTA := $(foreach dir,$(SRCDIRS_FLASH_MASTA),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES_FLASH_MASTA := $(foreach dir,$(SRCDIRS_FLASH_MASTA),$(notdir $(wildcard $(dir)/*.S)))
OBJECTS_FLASH_MASTA := $(CSOURCES_FLASH_MASTA:%.c=$(OBJDIR)/%.o) $(ASMSOURCES_FLASH_MASTA:%.S=$(OBJDIR)/%.o)

SRCDIRS_ALL := $(SRCDIRS) $(SRCDIRS_STUB) $(SRCDIRS_FLASH_MASTA)
CFLAGS += -Ires -Isrc

DEPS := $(OBJECTS:.o=.d)
CFLAGS += -MMD -MP

vpath %.c $(SRCDIRS_ALL)
vpath %.S $(SRCDIRS_ALL)

.PHONY: all clean install

all: CartFriend_FlashMasta.wsc CartFriend_FlashMasta_SafeMode.wsc CartFriend_FlashMasta.sram.bin CartFriend_Stub.ws

CartFriend_FlashMasta.wsc: $(OBJECTS) $(OBJECTS_FLASH_MASTA)
	$(SWANLINK) -v -o $@ --output-elf $@.elf $(SLFLAGS) --linker-args $(LDFLAGS) $(OBJECTS) $(OBJECTS_FLASH_MASTA) $(LIBS)

CartFriend_FlashMasta_SafeMode.wsc: $(OBJECTS) $(OBJECTS_FLASH_MASTA)
	$(SWANLINK) -v -o $@ --output-elf $@.elf $(SLFLAGS) --disable-custom-bootsplash --linker-args $(LDFLAGS) $(OBJECTS) $(OBJECTS_FLASH_MASTA) $(LIBS)

CartFriend_FlashMasta.sram.bin: $(OBJECTS) $(OBJECTS_FLASH_MASTA)
	$(SWANLINK) -v -o $@ --output-elf $@.elf -t sram_binary $(SLFLAGS) --linker-args $(LDFLAGS) $(OBJECTS) $(OBJECTS_FLASH_MASTA) $(LIBS)

CartFriend_Stub.ws: $(OBJECTS) $(OBJECTS_STUB)
	$(SWANLINK) -v -o $@ --output-elf $@.elf $(SLFLAGS) --linker-args $(LDFLAGS) $(OBJECTS) $(OBJECTS_STUB) $(LIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(info $(shell mkdir -p $(MKDIRS)))

clean:
	rm -r $(OBJDIR)/*
	rm CartFriend_FlashMasta.wsc CartFriend_FlashMasta.wsc.elf
	rm CartFriend_FlashMasta_SafeMode.wsc CartFriend_FlashMasta_SafeMode.wsc.elf
	rm CartFriend_Stub.ws CartFriend_Stub.ws.elf

-include $(DEPS)
