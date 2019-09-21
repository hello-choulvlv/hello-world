NDK=/Users/charlie/software/android-ndk-r13b
NDKABI=21
NDKVER=$NDK/toolchains/aarch64-linux-android-4.9
NDKP=$NDKVER/prebuilt/darwin-x86_64/bin/aarch64-linux-android-

NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm64"
NDKARCH="-march=armv8-a"

make HOST_CC="gcc -O2 -m64" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF $NDKARCH"
#make HOST_CC="gcc -O2 -m64" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF"