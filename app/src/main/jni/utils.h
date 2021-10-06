//
// Created by rostislav on 27.02.19.
//

#include <jni.h>
#include <android/log.h>

#ifndef CAMERAENGINENDK_UTILS_H
#define CAMERAENGINENDK_UTILS_H

typedef struct {
    int width;
    int height;
    int degree;
} Size;


// Wrapper for android LOG print

#include <android/log.h>

#define TAG "CameraEngineNDK"

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,    TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,     TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,     TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,    TAG, __VA_ARGS__)

#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, TAG, fmt, ##__VA_ARGS__); \
  }

#endif //CAMERAENGINENDK_UTILS_H
