#!/bin/bash
gcc -std=c99 -O0 -Wall -pedantic csc.c str_concat.c -o csc
gcc -std=c99 -O0 -Wall -pedantic sw.c str_concat.c -o sw
gcc -std=c99 -O0 -Wall -pedantic index.c str_concat.c -o index
