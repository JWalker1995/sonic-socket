#!/bin/sh

set -e

cd "$(dirname "$0")"

mkdir -p /usr/local/lib/
mkdir -p /usr/local/include/

rm -r /usr/local/include/libSonicSocket/
rm -r /usr/local/include/readerwriterqueue/

pushd build-$1/
cp libSonicSocket.a /usr/local/lib/
popd

pushd build-$1/src/
find . -name '*.h' -exec cp --parents '{}' /usr/local/include/ \;
popd

pushd src/
find . -name '*.h' -exec cp --parents '{}' /usr/local/include/ \;
popd

pushd 3rd_party/
cp --parents readerwriterqueue/readerwriterqueue.h readerwriterqueue/atomicops.h /usr/local/include/
popd
