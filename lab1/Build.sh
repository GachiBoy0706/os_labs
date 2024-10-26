#!/bin/bash
mkdir build
cd build
rm -rf *
cmake .. 
make

mv ./daemon ../ 
rm -rf *
mv ../daemon ./ 