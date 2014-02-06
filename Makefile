TARGET=avr_controller
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-Wall -std=c++11
LDFLAGS=
LDLIBS=-lm -lpthread 
ifeq ($(OS),Windows_NT)
	LDLIBS+=-lboost_system-mt -lboost_program_options-mt
	CPPFLAGS+=-U__STRICT_ANSI__
else
	LDLIBS+=-lboost_system -lboost_program_options
endif

PREFIX?=/usr

SRCS=main.cpp AVRController.cpp CommandConnection.cpp Daemon.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(TARGET)

install: $(TARGET)
	install -m 755 -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/$(TARGET)
	install -m 755 avr_command $(PREFIX)/bin/avr_command
	install -m 644 avr_controller.xml /etc/avr_controller.xml

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
