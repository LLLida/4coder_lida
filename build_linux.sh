#!/bin/bash

set -e

../bin/buildsuper_x64-linux.sh 4coder_lida.cpp
cp custom_4coder.so ../../../build/custom_4coder.so
