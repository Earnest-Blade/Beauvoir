@echo off
setlocal

set "BVR_GENERATOR=MinGW Makefiles"
set "BVR_CC=gcc"
set "BVR_CXX=g++"
set "BVR_BUILD_DIR=%cd%\build"
set "BVR_EXTERNAL_MODULES=SDL PortAudio Zlib Lpng"

rem ==== Clear command ====
if "%1"=="-clear" (
    echo cleaning...

    rmdir /S /Q "%cd%\bin"
    rmdir /S /Q "%cd%\lib"
    rmdir /S /Q "%cd%\licenses"
    rmdir /S /Q "%cd%\cmake"

    mkdir "%cd%\bin"
    mkdir "%cd%\lib"
)

rem ==== Loop through external modules ====
for %%M in (%BVR_EXTERNAL_MODULES%) do (
    set "MODULE_PATH=%cd%\extern\%%M"
    
    if exist "!MODULE_PATH!" (
        echo !MODULE_PATH! found!
        set "BVR_MODULE_FLAGS="

        if "%%M"=="PortAudio" (
            set "BVR_MODULE_FLAGS=-DPA_BUILD_SHARED_LIBS=ON -DPA_USE_WASAPI=OFF -DPA_USE_WDMKS=OFF -DPA_USE_WDMKS_DEVICE_INFO=OFF"
        )

        cmake "%MODULE_PATH%\CmakeLists.txt" -G "%BVR_GENERATOR%" -B "%BVR_BUILD_DIR%\%%M" -D CMAKE_INSTALL_PREFIX="%cd%" %BVR_MODULE_FLAGS% -DCMAKE_C_COMPILER=%BVR_CC% -DCMAKE_CXX_COMPILER=%BVR_CXX%
        cmake --build "%BVR_BUILD_DIR%\%%M" --target install
    ) else (
        echo !MODULE_PATH! not found!
    )
)

rem ==== Auto clear-up useless SDL folders ====
rmdir /S /Q "%cd%\licenses"
rmdir /S /Q "%cd%\cmake"

endlocal