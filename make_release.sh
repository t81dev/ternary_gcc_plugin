#!/bin/bash

# Build release tarball

VERSION=$(cat VERSION)
TARNAME="ternary-gcc-plugin-$VERSION"

mkdir -p release
cp -r . release/$TARNAME
cd release
tar -czf $TARNAME.tar.gz $TARNAME
echo "Release: $TARNAME.tar.gz"