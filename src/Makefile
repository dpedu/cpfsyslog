CC=gcc -Wall
CFLAGS=-g -I.
CFLAGS_STATIC=$(CCLAGS) --static
DEPS=
OBJ=main.o pfparser.o sysparser.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyslog: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -vf *.o csyslog

static: $(OBJ)
	gcc -o csyslog $^ $(CFLAGS_STATIC)

.PHONY: docker
docker: static
	docker build -t csyslog .
