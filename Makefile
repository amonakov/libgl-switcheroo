CFLAGS   ?= -Wall -O2 -march=native -g
CXXFLAGS ?= $(CFLAGS)

# On multilib systems, this needs to point to distribution-specific library
# subdir like in /usr (lib or lib64 for 64-bit, lib32 or lib for 32-bit)
LIBDIR   ?= lib

ALTPATH  := /usr/$$LIB/primus/libGL.so.1

CFLAGS += -DALTPATH='$(ALTPATH)'

all: $(LIBDIR)/libgl-switcheroo.so gtkglswitch

gtkglswitch: gtkglswitch.cpp
	$(CXX) $(CXXFLAGS) `pkg-config --cflags --libs gtk+-2.0` -o $@ $<

$(LIBDIR)/libgl-switcheroo.so: libgl-switcheroo.c
	mkdir -p $(LIBDIR)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<
