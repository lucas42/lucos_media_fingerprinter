#!/bin/sh

gcc -Wall -c fingerprinter.c -lchromaprint -lavcodec -lavformat && gcc -Wall -o fingerprinter fingerprinter.o -lchromaprint -lavcodec -lavformat -lsqlite3 -lavutil;
rm fingerprinter.o;
