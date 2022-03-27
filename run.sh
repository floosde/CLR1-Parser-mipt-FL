#!/usr/bin/bash

cd parser_tests
mkdir build
cd build
cmake ..
make
./parser_tests
cd ..
rm -r build
cd ..
mkdir build
cd build
cmake ..
make
./CLR1_parser
cd ..
rm -r build

