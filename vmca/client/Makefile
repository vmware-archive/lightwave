CC=gcc
CFLAGS=-I. -I../include -I /usr/include -I /opt/likewise/include -I ../idl
DEPS = hellomake.h

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)
