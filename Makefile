TARGET=avr_controller
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-O0 -D_DEBUG -g -Wall -std=c++11
LDFLAGS=
LDLIBS=-lm -lpthread 
ifeq ($(OS),Windows_NT)
	LDLIBS+=-lboost_system-mt -lboost_program_options-mt
	CPPFLAGS+=-U__STRICT_ANSI__
else
	LDLIBS+=-lboost_system -lboost_program_options
endif

SRCS=main.cpp AVRController.cpp CommandConnection.cpp Daemon.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(TARGET)

$(TARGET): pre.h.gch $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)
	
pre.h.gch: pre.h
	$(CXX) $(CPPFLAGS) pre.h

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .depend *.gch

include .depend
