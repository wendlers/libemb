##
# Toplevel Makefile
#
# Stefan Wendler, sw@kaltpost.de
##

BASEDIR 	= .
SRCDIR  	= src
BINDIR		= bin
FIRMWARE    = firmware.elf
DEPLOYDIR	= deploy
TMPDIR		= /tmp
VERSION		= 0.1
TARGET		= i2cslave_example_v$(VERSION)

# OOCD_IF    ?= interface/openocd-usb.cfg
OOCD_IF    ?= interface/flyswatter.cfg


ifeq ($(TARCH),STM32_100)
OOCD_BOARD ?= board/stm32100b_eval.cfg
else
OOCD_BOARD ?= board/olimex_stm32_h103.cfg
endif

all: target

world: target gen-docs

target:
	make -C $(SRCDIR)

gen-docs: target
	cd $(SRCDIR) && make gen-docs

style:
	cd $(SRCDIR) && make style

check:
	make -C $(SRCDIR) check

ifeq ($(TARCH),MSP430)
flash-target: target
	mspdebug rf2500 "prog $(BINDIR)/$(FIRMWARE)"
else
flash-target: target
	openocd -f $(OOCD_IF) -f $(OOCD_BOARD) \
                -c init -c targets -c "halt" \
                -c "flash write_image erase $(BINDIR)/$(FIRMWARE)" \
                -c "verify_image $(BINDIR)/$(FIRMWARE)" \
                -c "reset run" -c shutdown
endif

clean:
	make -C $(SRCDIR) clean
	rm -fr doc/gen
	rm -f bin/firmware.*
