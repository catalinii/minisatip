
CC=gcc
NODVBCSA=
#NODVBCSA=-DDISABLE_DVBCSA

CFLAGS= $(NODVBCSA) -ggdb -fPIC 
LDFLAGS=-lpthread -ldvbcsa -lrt

minisatip: minisatip.o socketworks.o stream.o dvb.o adapter.o dvbapi.o
	$(CC) $(CFLAGS) -o minisatip minisatip.o socketworks.o stream.o dvb.o dvbapi.o adapter.o $(LDFLAGS)

minisatip.o: minisatip.c minisatip.h socketworks.h stream.h
	$(CC) $(CFLAGS) -c -o minisatip.o minisatip.c 

socketworks.o: socketworks.c minisatip.h socketworks.h
	$(CC) $(CFLAGS) -c -o socketworks.o socketworks.c 

stream.o: stream.c minisatip.h socketworks.h stream.h adapter.h
	$(CC) $(CFLAGS) -c -o stream.o stream.c

dvb.o: dvb.c dvb.h
	$(CC) $(CFLAGS) -c -o dvb.o dvb.c

dvbapi.o: dvbapi.c dvbapi.h
	$(CC) $(CFLAGS) -c -o dvbapi.o dvbapi.c

adapter.o: adapter.c adapter.h dvb.h stream.h
	$(CC) $(CFLAGS) -c -o adapter.o adapter.c

all: minisatip

clean:
	rm *.o minisatip
