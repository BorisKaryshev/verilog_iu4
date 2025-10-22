#!/bin/bash

set -eou pipefail
set -x

base_path="$(pwd)"
for path in sem1 sem2 sem3 sem4 sem5; do
    cd $base_path/$path
    cmake -B build -S .
    cmake --build build
    ./build/exe
done
