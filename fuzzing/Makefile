# Fuzzing harness Makefile for Wormhole Portal plugin.
# Uses libFuzzer (clang) to fuzz the parameter parsing logic.

CC = clang
CFLAGS = -g -fsanitize=fuzzer,address -O1 -Wall -Wextra

.PHONY: all clean

all: run_fuzzer

run_fuzzer: fuzz_plugin.o
	$(CC) $(CFLAGS) -o $@ $^

fuzz_plugin.o: fuzz_plugin.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o run_fuzzer
