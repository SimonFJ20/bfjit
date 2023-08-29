#!/bin/bash

gcc -o program -std=c17 -Wall -Wextra -Wpedantic -O3 `cat compile_files.txt`

