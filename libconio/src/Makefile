LIBNAME	 = libconio
OBJS	+= conio.o conio_serial.o
INCDIR	+= -I../../libserial/src/include

include ../../common_lib.mk

check: $(SRC)
	$(CHECKER) $(CHECKERFLAGS) $(SRC)

gen-docs: $(HDR) $(SRC) 
	$(DOXYGEN) $(DOXYGENFLAGS)
