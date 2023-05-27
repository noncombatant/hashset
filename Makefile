CC = clang
CFLAGS = \
	-Weverything \
	-Werror \
	-std=c2x \
	-Wno-poison-system-directories \
	-Wno-declaration-after-statement

# Note: On Darwin, you might get "malloc: nano zone abandoned due to inability
# to reserve vm space." when running with Address Sanitizer. This warning is
# harmless: Darwin's libmalloc is telling you that it can't perform an
# optimization (see
# https://stackoverflow.com/questions/64126942/malloc-nano-zone-abandoned-due-to-inability-to-preallocate-reserved-vm-space).
# You can set `export MallocNanoZone=0` to suppress the warning.

debug: CFLAGS += -O0 -fsanitize=address -fsanitize=undefined -g
debug: LDFLAGS += -fsanitize=address -static-libsan
debug: run_test

release: CFLAGS += -O3 -flto=thin
release: run_test

run_test: test
	./test
	./test uniformity | sort -n

test: test.o util.o hashset.o

set.o: hashset.h hashset.c
test.o: test.c
util.o: util.h util.c

format:
	format-cc *.[ch]

clean:
	rm -f test
	rm -rf *.dSYM/
	rm -f *.o
