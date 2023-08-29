#!/bin/bash

set -xe

gcc -o program -std=c17 -Wall -Wextra -Wpedantic -O3 -g -fsanitize=address,undefined `cat compile_files.txt`

./program

