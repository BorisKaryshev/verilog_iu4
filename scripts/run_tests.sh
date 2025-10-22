#!/bin/bash

set -eou pipefail
set -x

base_path="$(pwd)"
for path in $(echo sem*); do
    cd $base_path/$path
    cmake -B build -S .
    cmake --build build
    ./build/exe
done
