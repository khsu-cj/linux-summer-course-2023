CFLAGS := -Wall -g -O2
LDFLAGS := -lpthread

ALL := test_pi_origin test_pi_fix

all: $(ALL)
.PHONY: all

test_pi_origin: main_pi_origin.c
	$(CC) $(CFLAGS) main_pi_origin.c -o $@ $(LDFLAGS)

test_pi_fix: main_pi_fix.c
	$(CC) $(CFLAGS) main_pi_fix.c -o $@ $(LDFLAGS)

test_pi_fix: CFLAGS += -I../include -DUSE_LINUX

origin: test_pi_origin
	@sudo taskset -c 1 ./test_pi_origin

fix: test_pi_fix
	@sudo taskset -c 1 ./test_pi_fix

run: all
	@echo "=== Running original test program ==="
	@sudo taskset -c 1 ./test_pi_origin
	@echo
	@echo "=== Running fixed test program ==="
	@sudo taskset -c 1 ./test_pi_fix

clean:
	$(RM) $(ALL)
.PHONY: clean