#!/bin/sh

CC=powerpc-buildroot-linux-uclibc-gcc \
  CFLAGS="-fPIC -Os" EMBEDDED=yes \
  DVBCSA=no DVBCA=no SATIPCLIENT=no make &&

powerpc-buildroot-linux-uclibc-strip minisatip
