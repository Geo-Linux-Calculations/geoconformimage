PNAME         = geoconformimage
PROGNAME      = $(PNAME)
CC            = gcc
CPP           = g++
CFLAGS        = -DUNIX -O2 -Wall -s
LIBS          = -lfreeimage
VER           = 0
VERB          = 20210324
ifeq ($(OS),Windows_NT)
PLIBF         = $(PNAME).$(VER).dll
PLIBFI        = $(PNAME)freeimage.$(VER).dll
RM            = del /Q
else
PLIBF         = lib$(PNAME).so.$(VER)
PLIBFI        = lib$(PNAME)freeimage.so.$(VER)
RM            = rm -f
endif
PLIB          = $(PLIBF) $(PLIBFI)
PREFIX        = /usr/local
DOCPREFIX     = $(PREFIX)/share/doc/$(PNAME)
SRCS          = src
INSTALL       = install
LN            = ln -fs

.PHONY: all clean install

all: $(PROGNAME)

clean:
	$(RM) $(PROGNAME) $(PLIBF) $(PLIBFI) *.exe $(SRCS)/*.o

$(SRCS)/geoconform.o: $(SRCS)/geoconform.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(SRCS)/geoconformfreeimage.o: $(SRCS)/geoconformfreeimage.cpp
	$(CPP) $(CFLAGS) -c $^ -o $@

$(SRCS)/geoconformimage.o: $(SRCS)/geoconformimage.cpp
	$(CPP) $(CFLAGS) -c $^ -o $@

$(PROGNAME): $(SRCS)/geoconform.o $(SRCS)/geoconformfreeimage.o $(SRCS)/geoconformimage.o
	$(CPP) $(CFLAGS) $^ -o $@ $(LIBS)

install: $(PROGNAME)
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -m 0755 $(PROGNAME) $(PREFIX)/bin/
	$(INSTALL) -d $(PREFIX)/share/man/man1
	$(INSTALL) -m 0644 man/man1/*.1 $(PREFIX)/share/man/man1
	$(INSTALL) -d $(PREFIX)/share/doc/$(PNAME)
	$(INSTALL) -m 0644 CHANGELOG LICENSE README.md VERSION $(PREFIX)/share/doc/$(PNAME)
