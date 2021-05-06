CC=gcc
CFLAGS=-Wall
TARGETS=GreyWolf.o network_setup.o keylogger_core.o
LIB_TARGETS=network_setup.o keylogger_core.o
FINAL_TARGETS=GreyWolf.o libWolf.a
all: libWolf.a GreyWolf #clean

libWolf.a:$(LIB_TARGETS)
	ar -rc $@ $^

GreyWolf: $(FINAL_TARGETS)
	$(CC) $^ -o $@

# .PHONY: clean
# clean:
# 	-rm -f *.o