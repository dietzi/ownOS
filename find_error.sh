#!/bin/sh

objdump -d kernel -S | grep -A 20 -B 10 $1
