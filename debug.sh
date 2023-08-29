#!/bin/bash

set -xe

gcc -o program main.c -std=c17 -Wall -Wextra -Wpedantic -O3 -g -fsanitize=address,undefined

./program

