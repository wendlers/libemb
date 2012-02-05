BINARY	 = firmware
OBJS	+= main.o 
INCDIR  += -I../../../libserial/src/include 
INCDIR  += -I../../../libconio/src/include
LIBDIR  += -L../../../libserial/lib 
LIBDIR  += -L../../../libconio/lib
LIBS	+= -lserial -lconio

ifeq ($(TARCH),STM32_100)
LDSCRIPT = ../../firmware_stm32_100.ld
CFLAGS   = -DSTM32_100
else
LDSCRIPT = ../../firmware_stm32_103.ld
CFLAGS   = -DSTM32_103
endif

include ../../../common.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC) 
	$(DOXYGEN) $(DOXYGENFLAGS)
