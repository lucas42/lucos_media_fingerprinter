#!/bin/sh

gcc -Wall -Wno-deprecated-declarations -c fingerprinter.c -lchromaprint -lavcodec -lavformat && gcc -Wall -o fingerprinter fingerprinter.o -lchromaprint -lavcodec -lavformat -lsqlite3 -lavutil;
rm fingerprinter.o;
