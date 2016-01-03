
CC?=gcc
EMBEDDED?=no
DVBCSA?=no #beat: re-enable later
DVBCA?=no
SATIPCLIENT?=yes
NETCVCLIENT?=yes
STATIC?=no

CFLAGS?=$(NODVBCSA) -ggdb -fPIC $(EXTRA_CFLAGS)
LDFLAGS?=-lpthread -lrt $(EXTRA_LDFLAGS)

OBJS=minisatip.o socketworks.o stream.o dvb.o adapter.o utils.o

TABLES=no

ifeq ($(DVBCSA),yes)
LIBS+=dvbcsa
OBJS+=dvbapi.o
TABLES=yes
else
CFLAGS+=-DDISABLE_DVBCSA
endif

ifeq ($(DVBCA),yes)
LIBS+=dvben50221 dvbapi ucsi
OBJS+=ca.o
TABLES=yes
else
CFLAGS+=-DDISABLE_DVBCA
endif

ifeq ($(TABLES),yes)
OBJS+=tables.o
endif

ifeq ($(SATIPCLIENT),yes)
OBJS+=satipc.o
else
CFLAGS+=-DDISABLE_SATIPCLIENT
endif

ifeq ($(NETCVCLIENT),yes)
OBJS+=netceiver.o
CFLAGS+=-I../vdr-mcli-plugin/mcast/client -I../vdr-mcli-plugin/mcast/common `xml2-config --cflags`
LDFLAGS+=-lmcli
else
CFLAGS+=-DDISABLE_NETCVCLIENT
endif

ifeq ($(EMBEDDED),yes)
CFLAGS+=-DNO_BACKTRACE
endif

ifeq ($(STATIC),yes)
LDFLAGS+=$(addsuffix .a,$(addprefix -l:lib,$(LIBS)))
else
LDFLAGS+=$(addprefix -l,$(LIBS))
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

satipc.o: satipc.c satipc.h
	$(CC) $(CFLAGS) -c -o $@ satipc.c

netceiver.o: netceiver.c netceiver.h
	$(CC) $(CFLAGS) -c -o $@ netceiver.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c -o $@ utils.c

all: minisatip

clean:
	rm *.o minisatip >> /dev/null
