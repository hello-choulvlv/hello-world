NDK=/Users/charlie/software/android-ndk-r13b
NDKABI=9
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv7-a -mfloat-abi=softfp -mfpu=neon -Wl,--fix-cortex-a8"
make HOST_CC="gcc -O2 -m32" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF $NDKARCH"

NDK=/Users/charlie/software/android-ndk-r10e
NDKARCH="-march=armv7-a -mfpu=neon -Wl,--fix-cortex-a8"

LUAJIT=./LuaJIT-2.0.2
cd $LUAJIT
NDK=/alex_data/tools/android-ndk-r8e
NDKABI=14
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.7
NDKP=$NDKVER/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv7-a -mfloat-abi=softfp -Wl,--fix-cortex-a8"
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF" TARGET_SYS=Other clean
make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Other

ISDKP=$(xcrun --sdk iphoneos --show-sdk-path)
ICC=$(xcrun --sdk iphoneos --find clang)
ISDKF="-arch arm64 -isysroot $ISDKP"
make DEFAULT_CC=clang HOST_CC="clang -m32 -arch i386" \
     CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS