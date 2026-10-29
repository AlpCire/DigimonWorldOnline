#!/usr/bin/env bash
set -euo pipefail
git config --global --add safe.directory /src
cmake -S /src -B /build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/dist \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DTOOLS_BUILD=none -DAPPS_BUILD=all -DSCRIPTS=static -DMODULES=static \
    -DWITH_WARNINGS=ON -DBUILD_TESTING=OFF
cmake --build /build --parallel "${DIGIMON_BUILD_JOBS:-6}"
cmake --install /build
