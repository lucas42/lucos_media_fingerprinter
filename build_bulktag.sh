#!/bin/sh

gcc -Wall -c bulktag.c && gcc -Wall -o bulktag bulktag.o -lsqlite3;
rm bulktag.o;
