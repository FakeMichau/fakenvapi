#!/bin/bash

set -e

shopt -s extglob

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: $0 version destdir"
  exit 1
fi

VERSION="$1"
SRC_DIR=$(dirname "$(readlink -f "$0")")
BUILD_DIR=$(realpath "$2")"/fakenvapi-$VERSION"

if [ -e "$BUILD_DIR" ]; then
  echo "Build directory $BUILD_DIR already exists"
  exit 1
fi

shift 2

crossfile="build-win"

function build_arch {
  cd "$SRC_DIR"

  meson setup                                \
    --cross-file "$SRC_DIR/$crossfile$1.txt" \
    --buildtype "release"                    \
    --prefix "$BUILD_DIR"                    \
    --strip                                  \
    --bindir "x$1"                           \
    --libdir "x$1"                           \
    "$BUILD_DIR/build.$1"

  cd "$BUILD_DIR/build.$1"
  ninja install
}

build_arch 64
build_arch 32
