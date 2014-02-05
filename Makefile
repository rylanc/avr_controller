CC=gcc
CXX=g++-4.7
RM=rm -f
#CPPFLAGS=-g $(shell root-config --cflags)
#LDFLAGS=-g $(shell root-config --ldflags)
#LDLIBS=$(shell root-config --libs)
CPPFLAGS=-O0 -D_DEBUG -g -Wall -std=c++11
LDFLAGS=
LDLIBS=-lm -lpthread -lboost_system -lboost_program_options

SRCS=main.cpp AVRController.cpp CommandConnection.cpp Daemon.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: avr_controller

avr_controller: $(OBJS)
	g++-4.7 $(LDFLAGS) -o avr_controller $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
