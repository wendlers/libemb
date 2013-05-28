LIBNAME	 = libi2c

ifeq ($(TARCH),MSP430)
OBJS	+= i2cslave_usci_msp430.o
else
OBJS	+= i2cslave_usart_stm32.o
endif

include ../../common_lib.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC)
	$(DOXYGEN) $(DOXYGENFLAGS)
