CC=gcc -Wall
CFLAGS=-g -I.
DEPS=
OBJ=main.o pfparser.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyslog: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)


clean:
	rm -vf *.o csyslog
