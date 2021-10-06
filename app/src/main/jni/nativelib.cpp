#include <jni.h>
#include <camera/NdkCameraManager.h>
#include <android/log.h>
#include <string>
#include <android/native_window_jni.h>
#include <media/NdkImageReader.h>
#include <thread>
#include "CameraUtils.h"

static ACameraManager *cameraManager = nullptr;
static ACameraDevice *cameraDevice = nullptr;
static ANativeWindow *textureWindow = nullptr;
static ACaptureRequest *request = nullptr;
static ACaptureSessionOutput *textureOutput = nullptr;
static ACaptureSessionOutputContainer *outputs = nullptr;
static ANativeWindow *imageWindow = nullptr;
static ACameraOutputTarget *imageTarget = nullptr;
static AImageReader *imageReader = nullptr;
static ACaptureSessionOutput *imageOutput = nullptr;
static ACameraOutputTarget *textureTarget = nullptr;
static ACameraCaptureSession *textureSession = nullptr;

static void onSessionActive(void *context, ACameraCaptureSession *session) {
}

static void onSessionReady(void *context, ACameraCaptureSession *session) {
}

static void onSessionClosed(void *context, ACameraCaptureSession *session) {
}

static ACameraCaptureSession_stateCallbacks sessionStateCallbacks{
        .context = nullptr,
        .onClosed = onSessionClosed,
        .onReady = onSessionReady,
        .onActive = onSessionActive
};

static void onDisconnected(void *context, ACameraDevice *device) {
}

static void onError(void *context, ACameraDevice *device, int error) {
}

static ACameraDevice_stateCallbacks cameraDeviceCallbacks = {
        .context = nullptr,
        .onDisconnected = onDisconnected,
        .onError = onError,
};

void onCaptureFailed(void *context, ACameraCaptureSession *session,
                     ACaptureRequest *req, ACameraCaptureFailure *failure) {
}

void onCaptureSequenceCompleted(void *context, ACameraCaptureSession *session,
                                int sequenceId, int64_t frameNumber) {}

void onCaptureSequenceAborted(void *context, ACameraCaptureSession *session,
                              int sequenceId) {}

void onCaptureCompleted(
        void *context, ACameraCaptureSession *session,
        ACaptureRequest *req, const ACameraMetadata *result) {
}

static ACameraCaptureSession_captureCallbacks captureCallbacks{
        .context = nullptr,
        .onCaptureStarted = nullptr,
        .onCaptureProgressed = nullptr,
        .onCaptureCompleted = onCaptureCompleted,
        .onCaptureFailed = onCaptureFailed,
        .onCaptureSequenceCompleted = onCaptureSequenceCompleted,
        .onCaptureSequenceAborted = onCaptureSequenceAborted,
        .onCaptureBufferLost = nullptr,
};

static void imageCallback(void *context, AImageReader *reader) {
    AImage *image = nullptr;
    auto status = AImageReader_acquireNextImage(reader, &image);
    std::thread processor([=]() {

        uint8_t *data = nullptr;
        int len = 0;
        AImage_getPlaneData(image, 0, &data, &len);

        AImage_delete(image);
    });
    processor.detach();
}

ANativeWindow *createSurface(AImageReader *reader) {
    ANativeWindow *nativeWindow;
    AImageReader_getWindow(reader, &nativeWindow);

    return nativeWindow;
}

AImageReader *createJpegReader() {
    AImageReader *reader = nullptr;
    media_status_t status = AImageReader_new(640, 480, AIMAGE_FORMAT_JPEG,
                                             4, &reader);
    AImageReader_ImageListener listener{
            .context = nullptr,
            .onImageAvailable = imageCallback,
    };
    AImageReader_setImageListener(reader, &listener);
    return reader;
}

void InitializeCamera(JNIEnv *env, jobject surface) {

    cameraManager = ACameraManager_create();
    auto id = GetBackFacingCameraId(cameraManager);
    ACameraManager_openCamera(cameraManager, id.c_str(), &cameraDeviceCallbacks, &cameraDevice);

    textureWindow = ANativeWindow_fromSurface(env, surface);
    ACameraDevice_createCaptureRequest(cameraDevice, TEMPLATE_PREVIEW, &request);

    ACaptureSessionOutput_create(textureWindow, &textureOutput);
    ACaptureSessionOutputContainer_create(&outputs);
    ACaptureSessionOutputContainer_add(outputs, textureOutput);

    imageReader = createJpegReader();
    imageWindow = createSurface(imageReader);
    ANativeWindow_acquire(imageWindow);
    ACameraOutputTarget_create(imageWindow, &imageTarget);
    ACaptureRequest_addTarget(request, imageTarget);
    ACaptureSessionOutput_create(imageWindow, &imageOutput);
    ACaptureSessionOutputContainer_add(outputs, imageOutput);

    ANativeWindow_acquire(textureWindow);
    ACameraOutputTarget_create(textureWindow, &textureTarget);
    ACaptureRequest_addTarget(request, textureTarget);

    ACameraDevice_createCaptureSession(cameraDevice, outputs, &sessionStateCallbacks,
                                       &textureSession);
    ACameraCaptureSession_setRepeatingRequest(textureSession, &captureCallbacks, 1, &request,
                                              nullptr);
}

extern "C"
JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_takePhoto(JNIEnv *env, jclass clazz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_stopCamera(JNIEnv *env, jclass clazz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_euphoria_psycho_knife_MainActivity_startCamera(JNIEnv *env, jclass clazz, jobject surface,
                                                    jint width, jint height) {
    InitializeCamera(env, surface);
}