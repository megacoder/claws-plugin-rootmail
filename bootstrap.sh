#!/bin/sh
rm -f configure
markdown2 README.md | tee README.html | lynx -stdin -dump >README
autoreconf -fvi
./configure
make
