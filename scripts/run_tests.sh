#!/bin/bash

set -eou pipefail
set -x

function run_test {
    target_dir="$1"
    cd "$1"
    cmake -B build -S .
    cmake --build build -j 4
    ./build/exe
    cd -
}

for i in sem*; do
    run_test $(realpath $i)
done
