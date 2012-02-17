LIBNAME		 = libnrf24l01
OBJS		+= nrf24l01.o nrf24l01_regs.o 


ifeq ($(TARCH),MSP430)
OBJS	+= nrf24l01_hw_msp430.o 
else
OBJS	+= nrf24l01_hw_stm32.o
endif


ifeq ($(WITH_META),1)
LIBNAME	  = libnrf24l01_meta
CFLAGS   += -DNRF_REG_DEF_META
endif

include ../../common_lib.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC)
	$(DOXYGEN) $(DOXYGENFLAGS)
