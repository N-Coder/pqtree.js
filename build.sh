#!/bin/bash

set -x
set -e

# source ../emsdk/emsdk_env.sh
git submodule update --init --recursive

pushd tourguide-js
rm -rf dist ../static/tourguide
npm install
npm run build
cp -r dist ../static/tourguide
popd


export CXXFLAGS="-fwasm-exceptions"

mkdir -p pctree-build
emmake cmake -S pctree -B pctree-build -DCMAKE_BUILD_TYPE=Debug
emmake make -C pctree-build PCTree

#mkdir -p pctreejs-build
#emmake cmake -S . -B pctreejs-build -DCMAKE_BUILD_TYPE=Release
#emmake make -C pctreejs-build

emcc wasm/glue.cpp wasm/layout.cpp wasm/drawing.cpp pctree/libraries/bigint/src/bigint.cpp \
  pctree-build/libPCTree.a -I pctree/include -I pctree/libraries/bigint/src \
  -lembind -o static/libPCTree.js -Og -g \
  -fwasm-exceptions
