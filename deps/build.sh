#!/bin/bash

set -e
set -x

cd json-c
./autogen.sh
./configure
make -j24
make install DESTDIR=$PWD/../build/
cd ../


# Note: in curl we've disabled the resolver and ssl. We have no DNS resolution or SSL support.
cd curl
./buildconf
./configure --enable-static=on --without-ssl --without-zlib --disable-shared --without-resolver
make -j24
make install DESTDIR=$PWD/../build/
cd ../
