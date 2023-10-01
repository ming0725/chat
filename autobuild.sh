#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi
rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
