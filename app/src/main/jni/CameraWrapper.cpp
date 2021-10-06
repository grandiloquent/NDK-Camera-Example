//
// Created by rostislav on 27.02.19.
//
#include <string>
#include "CameraWrapper.h"

CameraWrapper::CameraWrapper(JNIEnv *env, jobject pInstance) {
    isActive = true;
    env->GetJavaVM(&mJVM);
    mObjectRef = env->NewGlobalRef(pInstance);
    jclass cl = env->GetObjectClass(pInstance);
    previewFun = env->GetMethodID(cl, "imagePreview", "([III)Z");
}

JNIEnv* CameraWrapper::getJniEnv() {
    JavaVMAttachArgs attachArgs;
    attachArgs.version = JNI_VERSION_1_6;
    attachArgs.name = ">>>HelloThead";
    attachArgs.group = NULL;

    JNIEnv* env_n;
    if(mJVM->AttachCurrentThread(&env_n, &attachArgs) != JNI_OK)
    {
        env_n = NULL;
    }

    return env_n;
}

void CameraWrapper::imageGeting(cameraEngine* engine) {
    bitmapReady = true;
    while(1)
    {
        engine->drawFrame();
    }
}

void CameraWrapper::drawImage(jint *image, int w, int h) {
    if(bitmapReady) {
        bitmapReady = false;
        JNIEnv *env_n = getJniEnv();
        jlongArray1 = env_n->NewIntArray(h * w);

        if(jlongArray1 != NULL) {
            env_n->SetIntArrayRegion(jlongArray1, 0, h * w, image);
            bitmapReady = env_n->CallBooleanMethod(mObjectRef, previewFun, jlongArray1, w, h);
        } else {
            bitmapReady = true;
        }
        env_n->DeleteLocalRef(jlongArray1);
    }
}