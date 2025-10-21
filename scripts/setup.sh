#!/usr/bin/bash

set -eou pipefail
set -x

apt update
apt install -y make cmake g++ verilator
