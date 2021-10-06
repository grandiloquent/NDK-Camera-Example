//
// Created by rostislav on 27.02.19.
//

#ifndef CAMERAENGINENDK_CAMERAWRAPPER_H
#define CAMERAENGINENDK_CAMERAWRAPPER_H


#include <jni.h>
#include "cameraEngine.h"

class CameraWrapper {
public:
    CameraWrapper(JNIEnv *env, jobject pInstance);
    void imageGeting(cameraEngine* engine);
    void drawImage(jint *image, int w, int h);

private:
    JNIEnv* getJniEnv();
    jobject mObjectRef;
    JavaVM* mJVM;
    jmethodID previewFun;
    bool isActive;
    bool bitmapReady;
    jintArray jlongArray1;
};


#endif //CAMERAENGINENDK_CAMERAWRAPPER_H
