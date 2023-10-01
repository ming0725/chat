#!/bin/bash

mkdir build
rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
