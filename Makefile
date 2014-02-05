CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-O0 -D_DEBUG -U__STRICT_ANSI__-g -Wall -std=c++11
LDFLAGS=
ifeq ($(OS),Windows_NT)
	LDLIBS=-lm -lpthread -lboost_system-mt -lboost_program_options-mt
else
	LDLIBS=-lm -lpthread -lboost_system -lboost_program_options
endif

SRCS=main.cpp AVRController.cpp CommandConnection.cpp Daemon.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: avr_controller

avr_controller: $(OBJS)
	$(CXX) $(LDFLAGS) -o avr_controller $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
