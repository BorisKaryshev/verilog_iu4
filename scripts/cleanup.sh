#!/usr/bin/bash

find . -name build -type d | xargs rm -rf
find . -name main.vcd -type f | xargs rm -f
