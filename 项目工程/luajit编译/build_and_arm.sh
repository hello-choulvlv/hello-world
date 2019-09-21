NDK=/Users/charlie/software/android-ndk-r13b
NDKABI=9
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"

make HOST_CC="gcc -O2 -m32" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF"