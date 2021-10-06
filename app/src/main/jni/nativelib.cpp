
#include <jni.h>
#include <memory>
#include "camera_manager.h"
#include "image_reader.h"
#include <android/log.h>
#include <android/native_window_jni.h>
#include <media/NdkImageReader.h>

#define  LOG_TAG "native"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

jobject surface_;
JNIEnv *env_;
ANativeWindow *nativeWindow_;
std::unique_ptr<NDKCamera> camera_;
ImageFormat view_;
std::unique_ptr<ImageReader> jpgReader_;

extern "C" JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_startCamera(JNIEnv *env, jclass type,
                                                    jobject surface, jint width, jint height) {
    env_ = env;
    surface_ = env_->NewGlobalRef(surface);
    nativeWindow_ = ANativeWindow_fromSurface(env_, surface_);

    camera_.reset(new NDKCamera(ACAMERA_LENS_FACING_BACK));
    camera_->MatchCaptureSizeRequest(width, height, &view_);

//    ANativeWindow *showWindow_ = nullptr;
//    showWindow_ = ANativeWindow_fromSurface(env, surface);
//    ANativeWindow_setBuffersGeometry(showWindow_, view_.height, view_.width,
//                                     WINDOW_FORMAT_RGBA_8888);

    LOGE("==========> %dx%d %d", view_.width, view_.height, view_.width * 6);

    jpgReader_.reset(new ImageReader(view_.width * 4, view_.height * 4, AIMAGE_FORMAT_JPEG));
    jpgReader_->SetPresentRotation(0);
    jpgReader_->RegisterCallback(nullptr, [&](void *ctx, ImageReader *reader) -> void {
        int32_t format;


        AImage *image = reader->GetLatestImage();

        int32_t height = 0;
        AImage_getHeight(image, &height);

        int planeCount;
        media_status_t status = AImage_getNumberOfPlanes(image, &planeCount);
        uint8_t *data = nullptr;
        int len = 0;
        AImage_getPlaneData(image, 0, &data, &len);

        struct timespec ts{
                0, 0
        };
        clock_gettime(CLOCK_REALTIME, &ts);
        struct tm localTime;
        localtime_r(&ts.tv_sec, &localTime);

        std::string fileName = "/storage/emulated/0/Android/data/euphoria.psycho.knife/files/Pictures/";
        fileName += "capture" + std::to_string(localTime.tm_mon) +
                    std::to_string(localTime.tm_mday) + "-" +
                    std::to_string(localTime.tm_hour) +
                    std::to_string(localTime.tm_min) +
                    std::to_string(localTime.tm_sec) + ".jpg";
        FILE *file = fopen(fileName.c_str(), "wb");
        if (file && data && len) {
            fwrite(data, 1, len, file);
            fclose(file);
        } else {
            if (file)
                fclose(file);
        }
        AImage_delete(image);
    });
    //jpgReader_->WriteFile(jpgReader_->GetLatestImage());
    camera_->CreateSession(nativeWindow_, jpgReader_->GetNativeWindow(), false, 0);
    camera_->StartPreview(true);

}
extern "C"
JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_takePhoto(JNIEnv *env, jclass clazz) {
    camera_->TakePhoto();
}extern "C"
JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_stopCamera(JNIEnv *env, jclass clazz) {
    env_->DeleteGlobalRef(surface_);
    ANativeWindow_release(nativeWindow_);
    camera_.reset();
    jpgReader_.reset();
}