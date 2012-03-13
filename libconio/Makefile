##
# Toplevel Makefile 
#
# Stefan Wendler, sw@kaltpost.de 
##

BASEDIR 	= .
SRCDIR  	= src
DEPLOYDIR	= deploy
TMPDIR		= /tmp
VERSION		= 0.1
TARGET		= libconio_v$(VERSION)

# where to install to by default
ifeq ($(TARCH),MSP430)
INSTDIR		?= $(HOME)/msp430
else
INSTDIR		?= $(HOME)/sat/arm-none-eabi
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

install: target
	install -D -d -m 755 $(INSTDIR)/lib
	install -m 644 ./lib/*.a $(INSTDIR)/lib/.
	install -D -d -m 755 $(INSTDIR)/include/libemb/conio
	install -m 644 ./src/include/*.h $(INSTDIR)/include/libemb/conio/.

deploy-bin: clean target gen-docs 
	rm -fr $(TMPDIR)/$(TARGET)_bin
	mkdir $(TMPDIR)/$(TARGET)_bin
	cp ../LICENSE README $(TMPDIR)/$(TARGET)_bin/.
	cp -a ./lib $(TMPDIR)/$(TARGET)_bin/.
	cp -a ./doc $(TMPDIR)/$(TARGET)_bin/.
	cp -a ./src/include $(TMPDIR)/$(TARGET)_bin/.
	(cd $(TMPDIR); tar --exclude=".svn" -jcvf $(TARGET)_bin.tar.bz2  $(TARGET)_bin)
	mv $(TMPDIR)/$(TARGET)_bin.tar.bz2 $(DEPLOYDIR)/. 
	
deploy: deploy-bin

clean: 
	make -C $(SRCDIR) clean
	rm -fr doc/gen	
	rm -f lib/*.a
