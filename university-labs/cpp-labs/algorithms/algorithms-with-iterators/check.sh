#!/bin/bash

[[ $(grep -c -E '#include ["<]algorithm[">]' task.h) = 0 ]] || exit 1
[[ $(grep -c -E '#define' task.h) = 0 ]] || exit 1
