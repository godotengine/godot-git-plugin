#!/bin/bash

OPENSSL_VERSION="1.1.1s"

curl -OL http://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz
tar -xzvf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_arm64
tar -xzvf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_x86_64
cd openssl_arm64
./Configure darwin64-arm64-cc
make
cd ../
cd openssl_x86_64
./Configure darwin64-x86_64-cc -mmacosx-version-min=10.13
make
cd ../
lipo -create openssl_arm64/libcrypto.a openssl_x86_64/libcrypto.a -output thirdparty/openssl/libcrypto.a
lipo -create openssl_arm64/libssl.a openssl_x86_64/libssl.a -output thirdparty/openssl/libssl.a
rm openssl-$OPENSSL_VERSION.tar.gz
