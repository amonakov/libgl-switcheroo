CXX	 ?= g++
CXXFLAGS ?= -Wall -O2 -march=native -g

LIBPATH   := "/usr"
LIB64PATH := "/lib"
LIB32PATH := "/lib32"

CXXFLAGS += -DLIBPATH='$(LIBPATH)'

ifneq "$(LIB64PATH)" ""
CXXFLAGS += -DLIB64PATH='$(LIB64PATH)'
endif
ifneq "$(LIB32PATH)" ""
CXXFLAGS += -DLIB32PATH='$(LIB32PATH)'
endif

libgl-switcheroo: libgl-switcheroo.cpp
	$(CXX) $(CXXFLAGS) `pkg-config --cflags --libs fuse` -o $@ $<
