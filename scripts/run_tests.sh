#!/bin/bash

set -eou pipefail
set -x

function run_test {
    target_dir="$1"
    cd "$1"
    cmake -B build -S .
    cmake --build build -j 6
    ./build/exe
    cd -
}

export -f run_test

find semester_1 -maxdepth 1 | xargs -I{} -P 10 bash -c "run_test {}"
