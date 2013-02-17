#!/bin/bash
# Script for configuring libav for cross-compilation, with desired features.

./configure --prefix=$HOME/sshmusicplay.git/android-libs/libav \
    --enable-gpl \
    --enable-version3 \
    --disable-programs \
    --disable-swscale \
    --disable-avfilter \
    --disable-avresample \
    --disable-encoders \
    --disable-hwaccels \
    --disable-muxers \
    --disable-protocols \
    --disable-indevs \
    --disable-outdevs \
    --disable-asm \
    --disable-network \
    --enable-cross-compile \
    --target-os=linux \
    --arch=armv7-a \
    --cc=$ANDROID_STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-gcc \
    --nm=$ANDROID_STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-nm \
    --ar=$ANDROID_STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-ar
