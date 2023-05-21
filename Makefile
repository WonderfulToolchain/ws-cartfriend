ifndef WONDERFUL_TOOLCHAIN
$(error Please define WONDERFUL_TOOLCHAIN to point to the location of the Wonderful toolchain.)
endif
include $(WONDERFUL_TOOLCHAIN)/target/wswan/medium/makedefs.mk

TARGET ?= generic

OBJDIR := obj/$(TARGET)
MKDIRS := $(OBJDIR)
LIBS := -lwsx -lws
CFLAGS := $(WF_ARCH_CFLAGS) -I$(WF_TARGET_DIR)/include -Os -fno-jump-tables -ffunction-sections
LDFLAGS := $(WF_ARCH_LDFLAGS) -L$(WF_TARGET_DIR)/lib -Wl,--gc-sections

SRCDIRS := res src src/$(TARGET)
CSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES := $(foreach dir,$(SRCDIRS),$(notdir $(wildcard $(dir)/*.s)))
OBJECTS := $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.s=$(OBJDIR)/%.o)

CFLAGS += -Ires -Isrc -DTARGET_$(TARGET)

DEPS := $(OBJECTS:.o=.d)
CFLAGS += -MMD -MP

vpath %.c $(SRCDIRS)
vpath %.s $(SRCDIRS)

.PHONY: all clean install

all: CartFriend_$(TARGET).wsc

CartFriend_$(TARGET).wsc: $(OBJECTS) | $(OBJDIR)
	$(ROMLINK) -v -o $@ --output-elf $@.elf -- $(OBJECTS) $(LDFLAGS) $(LIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) -x assembler-with-cpp $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	$(info $(shell mkdir -p $(MKDIRS)))

clean:
	rm -r $(OBJDIR)/*
	rm CartFriend_$(TARGET).wsc CartFriend_$(TARGET).elf

-include $(DEPS)
