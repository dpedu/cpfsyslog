CC=cc
CFLAGS := -g -I. -Wall -Wpedantic -I ../deps/build/usr/local/include/
LDFLAGS = -L ../deps/build/usr/local/lib/
LDLIBS = -ljson-c -lcurl -lpthread
LDFLAGS += $(LDLIBS)
CFLAGS_STATIC = $(CFLAGS) --static
OBJ=main.o pfparser.o sysparser.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyslog: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -vf *.o csyslog

static: $(OBJ)
	$(CC) -o csyslog $^ $(LDFLAGS) --static

.PHONY: docker
docker: static
	sudo docker build -t csyslog .
