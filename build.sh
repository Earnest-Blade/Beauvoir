#! /bin/bash

BVR_GENERATOR="MinGW Makefiles"
BVR_BULID_DIR="$PWD/build/"
BVR_EXTERNAL_MODULES="SDL PortAudio Zlib Lpng"

#! clear command
if [ $1 = "-clear" ]; then 
    echo "cleaning..."

    rm -rf "$PWD/bin/"
    rm -rf "$PWD/lib/"

    rm -rf "$PWD/licenses"
    rm -rf "$PWD/cmake"

    mkdir "$PWD/bin/"
    mkdir "$PWD/lib/"
fi

for MOD in $BVR_EXTERNAL_MODULES; do 
    MODULE_PATH="$PWD/extern/$MOD"

    if [ -d "$MODULE_PATH" ]; then
        echo "$MODULE_PATH found!"
        BVR_MODULE_FLAGS="" 

        if [ "$MOD" = "PortAudio" ]; then 
            BVR_MODULE_FLAGS="PA_BUILD_SHARED_LIBS=ON PA_USE_WASAPI=OFF PA_USE_WDMKS=OFF PA_USE_WDMKS_DEVICE_INFO=OFF"
        else 
            BVR_MODULE_FLAGS=""
        fi

        cmake "$MODULE_PATH/CmakeLists.txt" -G="$BVR_GENERATOR" -B="$BVR_BULID_DIR/$MOD" -D CMAKE_INSTALL_PREFIX="$PWD" "$BVR_MODULE_FLAGS"
        cmake --build "$BVR_BULID_DIR/$MOD" --target install

    else 
        echo "$MODULE_PATH not found!"
    fi
done

#! auto clear-up useless SDL folders
rm -rf "$PWD/licenses"
rm -rf "$PWD/cmake"