CFLAGS   ?= -Wall -O2 -march=native -g
CXXFLAGS ?= $(CFLAGS)

LIBPATH   := "/usr"
LIB64PATH := "/lib"
LIB32PATH := "/lib32"
ALTPATH   := "/primus"


CFLAGS += -DLIBPATH='$(LIBPATH)'
CFLAGS += -DALTPATH='$(ALTPATH)'

ifneq "$(LIB64PATH)" ""
CFLAGS += -DLIB64PATH='$(LIB64PATH)'
endif
ifneq "$(LIB32PATH)" ""
CFLAGS += -DLIB32PATH='$(LIB32PATH)'
endif

all: libgl-switcheroo gtkglswitch

gtkglswitch: gtkglswitch.cpp
	$(CXX) $(CXXFLAGS) `pkg-config --cflags --libs gtk+-2.0` -o $@ $<

libgl-switcheroo: libgl-switcheroo.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs fuse` -o $@ $<
