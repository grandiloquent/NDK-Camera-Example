cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

add_library(nativelib SHARED
        cameraEngine.cpp
        cameraManager.cpp
        CameraWrapper.cpp
        imageReader.cpp
        jni_connect.cpp
        )

target_link_libraries(nativelib
        android
        camera2ndk
        mediandk
        log
        )