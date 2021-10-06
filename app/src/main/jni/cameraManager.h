//
// Created by rostislav on 27.02.19.
//
#include "imageReader.h"
#include <string>
#include <map>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraMetadataTags.h>
#include <vector>

#ifndef CAMERAENGINENDK_CAMERAMANAGER_H
#define CAMERAENGINENDK_CAMERAMANAGER_H

enum class CaptureSessionState : int32_t {
    READY = 0,  // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

struct CaptureRequestInfo {
    ANativeWindow* outputNativeWindow_;
    ACaptureSessionOutput* sessionOutput_;
    ACameraOutputTarget* target_;
    ACaptureRequest* request_;
    ACameraDevice_request_template template_;
    int sessionSequenceId_;
};

enum PREVIEW_INDICES {
    PREVIEW_REQUEST_IDX = 0,
    JPG_CAPTURE_REQUEST_IDX,
    CAPTURE_REQUEST_COUNT,
};

class DisplayDimension;

class CameraId {
public:
    ACameraDevice* device_;
    std::string id_;
    acamera_metadata_enum_android_lens_facing_t facing_;
    bool available_;  // free to use ( no other apps are using
    bool owner_;      // we are the owner of the camera
    explicit CameraId(const char* id)
            : device_(nullptr),
              facing_(ACAMERA_LENS_FACING_FRONT),
              available_(false),
              owner_(false) {
        id_ = id;
    }

    explicit CameraId(void) { CameraId(""); }
};

class cameraManager{
public:
    cameraManager(uint32_t facing_lens);
    ~cameraManager();

    bool firstPlay;

    void openCamera(void);
    void EnumerateCamera(void);
    bool maxCaptureSize(ImageFormat *preview, ImageFormat *view, DisplayDimension *perfectDimension);
    void CreateSession(ANativeWindow* previewWindow, ANativeWindow* captureWindow, int32_t imageRotation);
    std::vector<CaptureRequestInfo> requests_;
    void onDeviceState(ACameraDevice* device);
    void onDeviceError(ACameraDevice* device, int error);
    void onSessionState(ACameraCaptureSession* ses, CaptureSessionState state);
    void onCameraStatusChanged(const char* id, bool available);
    void startPreview(bool start);
    void TakePhoto();
    void returnImageSizes(jint *jpg, int s_jpg, jint *yuv, int s_yuv, jint *rgb, int s_rgb);

    void OnCaptureFailed(ACameraCaptureSession* session, ACaptureRequest* request, ACameraCaptureFailure* failure);
    void OnCaptureSequenceEnd(ACameraCaptureSession* session, int sequenceId, int64_t frameNumber);

private:
    ACameraManager* aCameraManager;
    std::map<std::string, CameraId> cameras_;
    std::string activeCameraId_;

    ACaptureSessionOutputContainer* outputContainer;
    ACameraCaptureSession* captureSession_;
    CaptureSessionState captureSessionState_;

    ACameraManager_AvailabilityCallbacks* GetManagerListener();
    ACameraDevice_stateCallbacks* GetDeviceListener();
    ACameraCaptureSession_stateCallbacks* GetSessionListener();
    ACameraCaptureSession_captureCallbacks* GetCaptureCallback();

    uint32_t facing_;
    bool valid_;
};


class DisplayDimension {
public:
    DisplayDimension(int32_t w, int32_t h) : w_(w), h_(h), portrait_(false) {
        if (h > w) {
            // make it landscape
            w_ = h;
            h_ = w;
            portrait_ = true;
        }
    }
    DisplayDimension(const DisplayDimension& other) {
        w_ = other.w_;
        h_ = other.h_;
        portrait_ = other.portrait_;
    }

    DisplayDimension(void) {
        w_ = 0;
        h_ = 0;
        portrait_ = false;
    }
    DisplayDimension& operator=(const DisplayDimension& other) {
        w_ = other.w_;
        h_ = other.h_;
        portrait_ = other.portrait_;

        return (*this);
    }

    bool IsDoublePort(){
        if(((w_ / h_) == 2) || ((h_/w_) == 2)){
            return true;
        } else{
            return false;
        }
    }

    bool IsSameRatio(DisplayDimension& other) {
        return (w_ * other.h_ == h_ * other.w_);
    }
    bool operator>(DisplayDimension& other) {
        return (w_ >= other.w_ & h_ >= other.h_);
    }
    bool operator==(DisplayDimension& other) {
        return (w_ == other.w_ && h_ == other.h_ && portrait_ == other.portrait_);
    }
    DisplayDimension operator-(DisplayDimension& other) {
        DisplayDimension delta(w_ - other.w_, h_ - other.h_);
        return delta;
    }
    void Flip(void) { portrait_ = !portrait_; }
    bool IsPortrait(void) { return portrait_; }
    int32_t getWidth() { return w_; }
    int32_t getHeight() { return h_; }
    int32_t org_width(void) { return (portrait_ ? h_ : w_); }
    int32_t org_height(void) { return (portrait_ ? w_ : h_); }

private:
    int32_t w_, h_;
    bool portrait_;
};


#endif //CAMERAENGINENDK_CAMERAMANAGER_H
