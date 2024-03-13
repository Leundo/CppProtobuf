#!/usr/bin/env bash

cd "$(dirname "$0")"

xcodebuild archive \
-scheme CppProtobuf \
-destination "generic/platform=iOS" \
-archivePath ./Build/CppProtobuf-iOS \
SKIP_INSTALL=NO \
BUILD_LIBRARY_FOR_DISTRIBUTION=YES

xcodebuild archive \
-scheme CppProtobuf \
-destination "generic/platform=iOS Simulator" \
-archivePath ./Build/CppProtobuf-Sim \
SKIP_INSTALL=NO \
BUILD_LIBRARY_FOR_DISTRIBUTION=YES

xcodebuild archive \
-scheme CppProtobuf \
-destination "generic/platform=OS X" \
-archivePath ./Build/CppProtobuf-OSX \
SKIP_INSTALL=NO \
BUILD_LIBRARY_FOR_DISTRIBUTION=YES

cd ./Build

xcodebuild -create-xcframework \
-framework ./CppProtobuf-iOS.xcarchive/Products/Library/Frameworks/CppProtobuf.framework \
-framework ./CppProtobuf-Sim.xcarchive/Products/Library/Frameworks/CppProtobuf.framework \
-framework ./CppProtobuf-OSX.xcarchive/Products/Library/Frameworks/CppProtobuf.framework \
-output ./CppProtobuf.xcframework