cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

add_library(nativelib SHARED
        camera_listeners.cpp
        camera_manager.cpp
        camera_utils.cpp
        image_reader.cpp
        nativelib.cpp
        )


target_link_libraries(nativelib
        android
        camera2ndk
        mediandk
        log
        )