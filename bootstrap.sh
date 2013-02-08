#!/bin/sh
rm -f configure
autoreconf -fvi
./configure
make
