CC=cc
override CFLAGS += -g -I. -Wall -Wpedantic -I ../deps/build/usr/local/include/ -std=gnu11
LIBPATH = ../deps/build/usr/local/lib/
LDFLAGS = -L $(LIBPATH)
LDLIBS = -ljson-c -lcurl -lpthread -lGeoIP
LDFLAGS += $(LDLIBS)
CFLAGS_STATIC = $(CFLAGS) --static
OBJ=helpers.o pfparser.o sysparser.o msgbuffer.o geo.o elasticsearch.o server.o vendor/argparse.o
TESTS=$(patsubst %.c,%.test,$(wildcard tests/*.c))


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyslog: $(OBJ) main.o
	$(CC) -o $@ $^ $(LDFLAGS)

static: $(OBJ) main.o
	$(CC) -o csyslog $^ $(LDFLAGS) --static

.PHONY: docker
docker: static
	sudo docker build -t csyslog .


.PHONY: clean
clean: cleantests
	rm -vf *.o csyslog

.PHONY: cleantests
cleantests:
	rm -vf tests/*.test


$(TESTS): tests/%.test: tests/%.c $(OBJ)
	$(CC) -o $@ $(OBJ) tests/$*.c $(CFLAGS) $(LDFLAGS)

.PHONY: tests
tests: $(TESTS)
	@bash -c 'for test in $^ ; do printf "======== %-30s ========\n" $$test ; LD_LIBRARY_PATH=$(LIBPATH) ./$$test ; printf "%48s\n" "RC: $$?" ; done'
