CFLAGS := -I../include -Wall -g -D_GNU_SOURCE -DUSE_LINUX -fsanitize=thread
LDFLAGS := -lpthread

ALL := qsort_mt

all: $(ALL)
.PHONY: all

qsort_mt: qsort_mt.c
	$(CC) $(CFLAGS) qsort_mt.c -o $@ $(LDFLAGS)

clean:
	$(RM) $(ALL)
.PHONY: clean