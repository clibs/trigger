AR ?= ar
CC ?= gcc
CFLAGS = -Ideps -lpthread -pedantic -std=c99 -v -Wall -Wextra

ifeq ($(APP_DEBUG),true)
	CFLAGS += -g -O0
else
	CFLAGS += -O2
endif

PREFIX ?= /usr/local

DEPS += $(wildcard deps/*/*.c)
SRCS += $(wildcard src/*.c)

OBJS += $(DEPS:.c=.o)
OBJS += $(SRCS:.c=.o)

all: build

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

build: build/lib/libtrigger.a
	mkdir -p build/include/trigger
	cp -f src/trigger.h build/include/trigger/trigger.h

build/lib/libtrigger.a: $(OBJS)
	mkdir -p build/lib
	$(AR) -crs $@ $^

clean:
	rm -fr *.o build deps/*/*.o example example.dSYM src/*.o

example: build
	$(CC) $(CFLAGS) -Ibuild/include -o example example.c -Lbuild/lib -ltrigger

install: all
	mkdir -p $(PREFIX)/include/trigger
	mkdir -p $(PREFIX)/lib
	cp -f src/trigger.h $(PREFIX)/include/trigger/trigger.h
	cp -f build/libtrigger.a $(PREFIX)/lib/libtrigger.a

uninstall:
	rm -fr $(PREFIX)/include/trigger/trigger.h
	rm -fr $(PREFIX)/lib/libtrigger.a

.PHONY: build clean example install uninstall
