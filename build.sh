#!/bin/bash

set -x
set -e

# git submodule update --init --recursive
# source ../emsdk/emsdk_env.sh

mkdir -p pctree-build
emmake cmake -S pctree -B pctree-build -DCMAKE_BUILD_TYPE=Debug
emmake make -C pctree-build PCTree

#mkdir -p pctreejs-build
#emmake cmake -S . -B pctreejs-build -DCMAKE_BUILD_TYPE=Release
#emmake make -C pctreejs-build

emcc wasm/glue.cpp wasm/layout.cpp wasm/drawing.cpp \
  pctree-build/libPCTree.a -I pctree/include \
  -lembind -o static/libPCTree.js -Og -g
