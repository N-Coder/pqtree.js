#!/bin/bash

set -x
set -e

# git submodule update --init --recursive
# source ../emsdk/emsdk_env.sh

mkdir -p pctree-build
emmake cmake -S pctree -B pctree-build -DCMAKE_BUILD_TYPE=Debug
emmake make -C pctree-build

#mkdir -p pctreejs-build
#emmake cmake -S . -B pctreejs-build -DCMAKE_BUILD_TYPE=Debug
#emmake make -C pctreejs-build

emcc wasm/glue.cpp \
  pctree-build/libPCTree.a -I pctree/include \
  -s EXPORTED_FUNCTIONS='["_Process"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]' -s EXPORT_NAME="'PQTreeModule'" \
  -s MODULARIZE -o wasm/libPCTree.js -O3
