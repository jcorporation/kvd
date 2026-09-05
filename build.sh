#!/bin/bash
#
# SPDX-License-Identifier: GPL-3.0-or-later
# (c) 2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/kvd

# Exit on errors
set -eEo pipefail

SCRIPT_PATH=$(dirname "$(realpath "$0")")

build_debug() {
    echo "Debug build"
    install -d build
    cmake -B build \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        .
    make -j4 -C build
    echo "Linking compilation database"
    sed -e 's/\\t/ /g' -e 's/-Wformat-truncation//g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' \
        -e 's/-Wno-stringop-overread//g' -e 's/-fstack-clash-protection//g' \
        build/compile_commands.json > src/compile_commands.json
}

build_release() {
    echo "Release build"
    install -d build
    cmake -B build \
        -DCMAKE_INSTALL_PREFIX:PATH=/usr \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        .
    make -j4 -C build
    echo "Linking compilation database"
    sed -e 's/\\t/ /g' -e 's/-Wformat-truncation//g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' \
        -e 's/-Wno-stringop-overread//g' -e 's/-fstack-clash-protection//g' \
        build/compile_commands.json > src/compile_commands.json
}

lint() {
    echo "Running clang-tidy"
    rm -f clang-tidy.out
    cd src || exit 1
    find ./ -name '*.c' -exec clang-tidy \
      --config-file="$SCRIPT_PATH/.clang-tidy" {} \; >> ../clang-tidy.out 2>/dev/null
    ERRORS=$(grep -v -E "(memset|memcpy|\^)" ../clang-tidy.out)
    if [ -n "$ERRORS" ]
    then
      echo "$ERRORS"
      return 1
    fi
}

case "$1" in
    debug)
        build_debug
        ;;
    release)
        build_release
        ;;
    lint)
        lint
        ;;
    *)
        echo "Usage: $(basename "$0") <debug|release|lint>"
        exit 1
        ;;
esac
