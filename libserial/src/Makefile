LIBNAME	 = libserial
OBJS	+= serial_rb.o 

ifeq ($(TARCH),MSP430)
OBJS	+= serial_usci_msp430.o
else
OBJS	+= serial_usart_stm32.o
endif

include ../../common_lib.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC)
	$(DOXYGEN) $(DOXYGENFLAGS)
