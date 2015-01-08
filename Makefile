CC=gcc
CFLAGS= -ggdb -fPIC 
LDFLAGS=-lpthread -lrt

minisatip: minisatip.o socketworks.o stream.o dvb.o adapter.o
	$(CC) $(CFLAGS) -o minisatip minisatip.o socketworks.o stream.o dvb.o adapter.o $(LDFLAGS)
minisatip.o: minisatip.c minisatip.h socketworks.h stream.h
	$(CC) $(CFLAGS) -c -o minisatip.o minisatip.c 

socketworks.o: socketworks.c minisatip.h socketworks.h
	$(CC) $(CFLAGS) -c -o socketworks.o socketworks.c 

stream.o: stream.c minisatip.h socketworks.h stream.h
	$(CC) $(CFLAGS) -c -o stream.o stream.c

dvb.o: dvb.c dvb.h
	$(CC) $(CFLAGS) -c -o dvb.o dvb.c

adapter.o: adapter.c adapter.h
	$(CC) $(CFLAGS) -c -o adapter.o adapter.c

ssh:	minisatip	
	scp *c *h root@16.179.162.52:/jail/Users/cata/scripts/minisatip/
	ssh root@16.179.162.52 "(cd /jail/Users/cata/scripts/minisatip/;export STAGING_DIR=.;make all)"
all: minisatip
