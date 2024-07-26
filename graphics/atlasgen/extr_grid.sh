#!/bin/sh
g++ main_spritegen.cpp
./a.out
cp alt.res ../build/all.res

cd ../build
./geometry

