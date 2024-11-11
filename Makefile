# -fsanitize=... documentation:
#   https://gcc.gnu.org/onlinedocs/gcc-11.4.0/gcc/Instrumentation-Options.html
# 	https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer

CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=cdict_test


all: $(TARGETS)

cdict_test : cdict.c cdict.h cdict_test.c
	gcc $(CFLAGS) $^ -o $@


clean:
	rm -f $(TARGETS)
