#!/bin/sh
android-cmake ../libssh-0.5.3 -DOPENSSL_LIBRARIES="$PWD/../android-libs/openssl/lib/libssl.so;$PWD/../android-libs/openssl/lib/libcrypto.so" -DOPENSSL_INCLUDE_DIRS=$PWD/../android-libs/openssl/include -DANDROID=1 -DCMAKE_INSTALL_PREFIX=$PWD/../android-libs/libssh
