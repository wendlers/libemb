##
# Toplevel Makefile 
#
# Stefan Wendler, sw@kaltpost.de 
##

BASEDIR 	= .
BINDIR		= bin
SRCDIR  	= src
DEPLOYDIR	= deploy
FIRMWARE    = firmware.elf
TMPDIR		= /tmp
VERSION		= 0.1
TARGET		= nrf24l01_sample_server_v$(VERSION)

#OOCD_IF    ?= interface/openocd-usb.cfg 
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

deploy-src: target check style clean gen-docs
	rm -fr $(TMPDIR)/$(TARGET)_src
	mkdir $(TMPDIR)/$(TARGET)_src
	cp -a ./* $(TMPDIR)/$(TARGET)_src/.
	rm -fr $(TMPDIR)/$(TARGET)_src/deploy
	rm -fr $(TMPDIR)/$(TARGET)_src/tools
	mkdir $(TMPDIR)/$(TARGET)_src/deploy
	(cd $(TMPDIR); tar --exclude=".svn" -jcvf $(TARGET)_src.tar.bz2  $(TARGET)_src)
	mv $(TMPDIR)/$(TARGET)_src.tar.bz2 $(DEPLOYDIR)/. 

deploy-bin: clean target 
	rm -fr $(TMPDIR)/$(TARGET)_bin
	mkdir $(TMPDIR)/$(TARGET)_bin
	cp CHANGELOG LICENSE README $(TMPDIR)/$(TARGET)_bin/.
	cp -a ./bin $(TMPDIR)/$(TARGET)_bin/.
	cp -a ./doc $(TMPDIR)/$(TARGET)_bin/.
	(cd $(TMPDIR); tar --exclude=".svn" -jcvf $(TARGET)_bin.tar.bz2  $(TARGET)_bin)
	mv $(TMPDIR)/$(TARGET)_bin.tar.bz2 $(DEPLOYDIR)/. 
	
deploy: deploy-src deploy-bin

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
