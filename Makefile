
CC?=gcc
DVBCSA?=yes
DVBCA?=no

CFLAGS=$(NODVBCSA) -ggdb -fPIC
LDFLAGS=-lpthread -lrt

OBJS=minisatip.o socketworks.o stream.o dvb.o adapter.o tables.o

ifeq ($(DVBCSA),yes)
LDFLAGS+=-ldvbcsa
OBJS+=dvbapi.o
else
CFLAGS+=-DDISABLE_DVBCSA
endif

ifeq ($(DVBCA),yes)
LDFLAGS+=-ldvben50221 -ldvbapi -lucsi
OBJS+=ca.o
else
CFLAGS+=-DDISABLE_DVBCA
endif


minisatip: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

minisatip.o: minisatip.c minisatip.h socketworks.h stream.h
	$(CC) $(CFLAGS) -c -o $@ minisatip.c 

socketworks.o: socketworks.c minisatip.h socketworks.h
	$(CC) $(CFLAGS) -c -o $@ socketworks.c 

stream.o: stream.c minisatip.h socketworks.h stream.h adapter.h
	$(CC) $(CFLAGS) -c -o $@ stream.c

dvb.o: dvb.c dvb.h
	$(CC) $(CFLAGS) -c -o $@ dvb.c

dvbapi.o: dvbapi.c dvbapi.h
	$(CC) $(CFLAGS) -c -o $@ dvbapi.c

adapter.o: adapter.c adapter.h dvb.h stream.h ca.h
	$(CC) $(CFLAGS) -c -o $@ adapter.c

ca.o: ca.c adapter.h dvb.h ca.h
	$(CC) $(CFLAGS) -c -o $@ ca.c
tables.o: tables.c tables.h
	$(CC) $(CFLAGS) -c -o $@ tables.c

all: minisatip

clean:
	rm *.o minisatip
