CC=gcc -Wall
CFLAGS=-g -I.
DEPS=
OBJ=main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

csyslog: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
