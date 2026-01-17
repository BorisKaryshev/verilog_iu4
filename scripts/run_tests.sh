#!/bin/bash

set -eou pipefail
set -x

build ()
{
    cd "$1"
    cmake -B build -S .
    cmake --build build -j 6
    cd -
}

run_test ()
{
    target_dir="$1"
    cd "$1"
    ./build/exe
    cd -
}

export -f build

projects=$(echo -e "                     \
                 semester_1/sem2_task2\n \
                 semester_1/sem3\n       \
                 semester_1/sem4\n       \
                 semester_1/sem5\n       \
                 semester_2/task_1\n       \
                 semester_2/task_2\n       \
")

printf '%s' "$projects" | xargs -I{} -P 10 bash -c "set -x; build {}"

for i in $(echo $projects); do
    run_test "$i"
done
