#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LIPO="xcrun -sdk iphoneos lipo"
STRIP="xcrun -sdk iphoneos strip"

SRCDIR=$DIR/src
DESTDIR=$DIR/lib
IXCODE=`xcode-select -print-path`
INFOPLIST_PATH=$IXCODE/Platforms/iPhoneOS.platform/version.plist
BUNDLE_ID=$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${INFOPLIST_PATH}")

mkdir "$DESTDIR"
rm "$DESTDIR"/*.a

# Config path for simulator
ISDK=$IXCODE/Platforms/iPhoneSimulator.platform/Developer
ISDKVER=iPhoneSimulator${BUNDLE_ID}.sdk
ISDKP=/usr/bin/

# i386 [for simulator]
ISDKF="-arch i386 -mios-simulator-version-min=9.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
mv "$SRCDIR"/libluajit.a "$DESTDIR"/libluajit-i386.a

# x86_64 [for simulator]
ISDKF="-arch x86_64 -mios-simulator-version-min=9.0 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="gcc -m64 -arch x86_64" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
mv "$SRCDIR"/libluajit.a "$DESTDIR"/libluajit-x86_64.a

# Config path for device
ISDK=$IXCODE/Platforms/iPhoneOS.platform/Developer
ISDKVER=iPhoneOS${BUNDLE_ID}.sdk
ISDKP=$IXCODE/usr/bin/

# armv7
make clean
ISDKF="-arch armv7 -mios-simulator-version-min=9.0 -isysroot $ISDK/SDKs/$ISDKVER"
make HOST_CC="gcc -m32 -arch i386" TARGET_FLAGS="$ISDKF" TARGET=arm TARGET_SYS=iOS
mv "$SRCDIR"/libluajit.a "$DESTDIR"/libluajit-armv7.a

# armv7s
make clean
ISDKF="-arch armv7s -mios-simulator-version-min=9.0 -isysroot $ISDK/SDKs/$ISDKVER"
make HOST_CC="gcc -m32 -arch i386" TARGET_FLAGS="$ISDKF" TARGET=arm TARGET_SYS=iOS
mv "$SRCDIR"/libluajit.a "$DESTDIR"/libluajit-armv7s.a

# arm64
make clean
ISDKF="-arch arm64 -mios-simulator-version-min=9.0 -isysroot $ISDK/SDKs/$ISDKVER"
make HOST_CC="gcc" TARGET_FLAGS="$ISDKF" TARGET=arm64 TARGET_SYS=iOS
mv "$SRCDIR"/libluajit.a "$DESTDIR"/libluajit-arm64.a

# Glue all library to one
$LIPO -create "$DESTDIR"/libluajit-*.a -output "$DESTDIR"/libluajit.a
$STRIP -S "$DESTDIR"/libluajit.a
$LIPO -info "$DESTDIR"/libluajit.a
