cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

file(GLOB SRC_FILES
        "*.cpp")

add_library(nativelib SHARED ${SRC_FILES})

target_link_libraries(nativelib
        android
        camera2ndk
        mediandk
        log
        )