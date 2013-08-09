##
# Common settings for Makefiles
#
# Stefan Wendler, sw@kaltpost.de
##
#
# compiler prefix
ifeq ($(TARCH),MSP430)
PREFIX  ?= msp430-
else
PREFIX	?= arm-none-eabi-
endif

CC		 = $(PREFIX)gcc
LD		 = $(PREFIX)gcc
OBJCOPY		 = $(PREFIX)objcopy
OBJDUMP		 = $(PREFIX)objdump

ifeq ($(TARCH),MSP430)
INCDIR		+= -I./include 
CFLAGS		+= -Os -g -mmcu=msp430g2553 -Wall -Wextra $(INCDIR) 
LDFLAGS     	+= -mmcu=msp430g2553 $(LIBDIR) $(LIBS)
else
INCDIR		+= -I./include -I$(HOME)/sat/arm-none-eabi/include
CFLAGS		+= -Os -g -Wall -Wextra -fno-common -mcpu=cortex-m3 -mthumb -msoft-float -MD $(INCDIR) -DSTM32F1
LDSCRIPT	?= $(BINARY).ld
LIBDIR		+= -L$(HOME)/sat/arm-none-eabi/lib -L$(HOME)/sat/arm-none-eabi/lib/stm32/f1
LIBS		+= -lopencm3_stm32f1
LDFLAGS		+= $(LIBDIR) $(LIBS) -T$(LDSCRIPT) -nostartfiles -Wall -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float
endif

# where to put generated binaries to
BINDIR		?= ../bin

# doxygen executable
DOXYGEN = doxygen

# doxygen flags
DOXYGENFLAGS = ../doxygen.conf

# styler to use
STYLER = astyle

# which c-style to use
# - see man astyle
STYLERFLAGS = --style=stroustrup

# cpp checker
CHECKER = cppcheck

# flags for checker
# CHECKERFLAGS = --error-exitcode=1 --enable=all
CHECKERFLAGS = --enable=all --error-exitcode=1

.SUFFIXES: .elf .bin .hex .srec .list .images
.SECONDEXPANSION:
.SECONDARY:

all: images

images: $(BINARY).images

%.images: %.bin %.hex %.srec %.list
	@#echo "*** $* images generated ***"

%.bin: %.elf
	@#printf "  OBJCOPY $(*).bin\n"
	$(OBJCOPY) -Obinary $(*).elf $(*).bin && cp $(*).bin $(BINDIR)/.

%.hex: %.elf
	@#printf "  OBJCOPY $(*).hex\n"
	$(OBJCOPY) -Oihex $(*).elf $(*).hex && cp $(*).hex $(BINDIR)/.

%.srec: %.elf
	@#printf "  OBJCOPY $(*).srec\n"
	$(OBJCOPY) -Osrec $(*).elf $(*).srec && cp $(*).srec $(BINDIR)/.

%.list: %.elf
	@#printf "  OBJDUMP $(*).list\n"
	$(OBJDUMP) -S $(*).elf > $(*).list && cp $(*).list $(BINDIR)/.

%.elf: $(OBJS) $(LDSCRIPT)
	@#printf "  LD      $(subst $(shell pwd)/,,$(@))\n"
	$(LD) $(OBJS) $(LDFLAGS) -o $(*).elf && cp $(*).elf $(BINDIR)/.

%.o: %.c Makefile
	@#printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp Makefile
	@#printf "  CC      $(subst $(shell pwd)/,,$(@))\n"
	$(CC) $(CFLAGS) -o $@ -c $<

SRC = $(wildcard *.c)
HDR = $(wildcard include/*.h)

style:
	$(STYLER) $(STYLERFLAGS) $(SRC)
	$(STYLER) $(STYLERFLAGS) $(HDR)

clean:
	rm -f *.o
	rm -f *.d
	rm -f *.elf
	rm -f *.bin
	rm -f *.hex
	rm -f *.srec
	rm -f *.list
	rm -f *.orig

.PHONY: images clean

