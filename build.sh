#!/bin/bash
mkdir release
pushd release
cmake .. -DCMAKE_BUILD_TYPE=Release
make
popd
mkdir debug
pushd debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
popd