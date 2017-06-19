CC=gcc
LDFLAGS=-fPIC -shared
TASKPATH=.

#all:test libiid.so

test:test.c libiid.so
	$(CC) -o $@ $^ -L$(TASKPATH) -liid

libiid.so:iid.o
	$(CC) $(LDFLAGS) -o $@ iid.c
