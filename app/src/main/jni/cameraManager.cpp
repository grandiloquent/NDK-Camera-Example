//
// Created by rostislav on 27.02.19.
//
#include <thread>

#include <camera/NdkCameraMetadata.h>
#include "cameraManager.h"
#include "utils.h"

// Create cameraManage for manipulation with camera device
cameraManager::cameraManager(uint32_t facing_lens):
    aCameraManager(nullptr),
    activeCameraId_(""),
    outputContainer(nullptr),
    facing_(facing_lens)
{
    valid_ = false;
    requests_.resize(CAPTURE_REQUEST_COUNT);
    memset(requests_.data(), 0, requests_.size() * sizeof(requests_[0]));
    cameras_.clear();
    aCameraManager = ACameraManager_create();
    EnumerateCamera();
    firstPlay = true;
}

void cameraManager::openCamera() {
    ACameraManager_openCamera(aCameraManager, activeCameraId_.c_str(), GetDeviceListener(), &cameras_[activeCameraId_].device_);

    ACameraManager_registerAvailabilityCallback(aCameraManager, GetManagerListener());

    valid_ = false;
}

// Enumerate all camera in this device
// Create array cameras_
void cameraManager::EnumerateCamera() {
    ACameraIdList* cameraIdList = nullptr;
    ACameraManager_getCameraIdList(aCameraManager, &cameraIdList);
    bool isCameraCheck = false;

    for(int i = 0; i < cameraIdList->numCameras; i++)
    {
        const char *id = cameraIdList->cameraIds[i];
        ACameraMetadata *metaObj;
        ACameraManager_getCameraCharacteristics(aCameraManager, id, &metaObj);
        int32_t count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(metaObj, &count, &tags);
        for(int tagIdx = 0; tagIdx < count; tagIdx++)
        {
            if(ACAMERA_LENS_FACING == tags[tagIdx])
            {
                ACameraMetadata_const_entry lensinfo = {0,};
                ACameraMetadata_getConstEntry(metaObj, tags[tagIdx], &lensinfo);
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(lensinfo.data.u8[0]);
                cam.owner_ = false;
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if(cam.facing_ == facing_ && isCameraCheck == false){
                    activeCameraId_ = cam.id_;
                    isCameraCheck = true;
                    break;
                }
                break;
            }
        }
        ACameraMetadata_free(metaObj);
    }
    if(activeCameraId_.length() == 0){
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIdList);
}

// Function for load and choise maximum size for preview
bool cameraManager::maxCaptureSize(ImageFormat *preview, ImageFormat *capture, DisplayDimension *perfectDimension) {
    ACameraMetadata* metadata;
    ACameraManager_getCameraCharacteristics(aCameraManager, activeCameraId_.c_str(), &metadata);
    ACameraMetadata_const_entry metadata_const_entry;
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &metadata_const_entry);

    bool foundIt = false;
    bool foundJpg = false;

    DisplayDimension foundRes(0,0), maxCapture(0,0);

    for(int i = 0; i < metadata_const_entry.count; i++)
    {
        int32_t input = metadata_const_entry.data.i32[i+3];
        int32_t format = metadata_const_entry.data.i32[i+0];
        if(input) continue;

        if(format == AIMAGE_FORMAT_YUV_420_888 || format == AIMAGE_FORMAT_JPEG)
        {
            DisplayDimension res(metadata_const_entry.data.i32[i+1],
                                 metadata_const_entry.data.i32[i+2]);
            if(format == AIMAGE_FORMAT_YUV_420_888 && (*perfectDimension) > res && !foundIt){
                foundIt = true;
                foundRes = res;
            }
            if((format == AIMAGE_FORMAT_YUV_420_888 || res > maxCapture) && !foundJpg && res.IsDoublePort()){
                maxCapture = res;
                foundJpg = true;
            }
        }
    }



    if(foundIt)
    {
        preview->width = foundRes.getWidth();
        preview->height = foundRes.getHeight();
        capture->width = maxCapture.getWidth();
        capture->height = maxCapture.getHeight();
    } else{
        preview->width = 480;
        preview->height = 640;
        *capture = *preview;
    }

    preview->format = AIMAGE_FORMAT_YUV_420_888;
    capture->format = AIMAGE_FORMAT_JPEG;

    firstPlay = false;
    return foundIt;
}

// Create camera session
void cameraManager::CreateSession(ANativeWindow* previewWindow,
                                  ANativeWindow* captureWindow, int32_t imageRotation) {
    // Create output from this app's ANativeWindow, and add into output container
    requests_[PREVIEW_REQUEST_IDX].template_ = TEMPLATE_PREVIEW;
    requests_[PREVIEW_REQUEST_IDX].outputNativeWindow_ = previewWindow;
    requests_[JPG_CAPTURE_REQUEST_IDX].template_ = TEMPLATE_STILL_CAPTURE;
    requests_[JPG_CAPTURE_REQUEST_IDX].outputNativeWindow_ = captureWindow;

    ACaptureSessionOutputContainer_create(&outputContainer);
    for (auto& req : requests_) {
        ANativeWindow_acquire(req.outputNativeWindow_);
        ACaptureSessionOutput_create(req.outputNativeWindow_, &req.sessionOutput_);
        ACaptureSessionOutputContainer_add(outputContainer, req.sessionOutput_);
        ACameraOutputTarget_create(req.outputNativeWindow_, &req.target_);
        ACameraDevice_createCaptureRequest(cameras_[activeCameraId_].device_,
                                      req.template_, &req.request_);
        ACaptureRequest_addTarget(req.request_, req.target_);
    }

    // Create a capture session for the given preview request
    captureSessionState_ = CaptureSessionState::READY;
    auto a = GetSessionListener();
    ACameraDevice_createCaptureSession(cameras_[activeCameraId_].device_,
                                  outputContainer, a,
                                  &captureSession_);

    ACaptureRequest_setEntry_i32(requests_[JPG_CAPTURE_REQUEST_IDX].request_,
                                 ACAMERA_JPEG_ORIENTATION, 1, &imageRotation);

    /*
     * Only preview request is in manual mode, JPG is always in Auto mode
     * JPG capture mode could also be switch into manual mode and control
     * the capture parameters, this sample leaves JPG capture to be auto mode
     * (auto control has better effect than author's manual control)
     */
    uint8_t aeModeOff = ACAMERA_CONTROL_AE_MODE_ON;
    ACaptureRequest_setEntry_u8(requests_[PREVIEW_REQUEST_IDX].request_,
                             ACAMERA_CONTROL_AE_MODE, 1, &aeModeOff);
    /*CALL_REQUEST(setEntry_i32(requests_[PREVIEW_REQUEST_IDX].request_,
                              ACAMERA_SENSOR_SENSITIVITY, 1, &sensitivity_));
    CALL_REQUEST(setEntry_i64(requests_[PREVIEW_REQUEST_IDX].request_,
                              ACAMERA_SENSOR_EXPOSURE_TIME, 1, &exposureTime_));*/
}

// Delete camera manager
cameraManager::~cameraManager() {
    valid_ = false;
    // stop session if it is on:
    if (captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }
    ACameraCaptureSession_close(captureSession_);

    for (auto& req : requests_) {
        ACaptureRequest_removeTarget(req.request_, req.target_);
        ACaptureRequest_free(req.request_);
        ACameraOutputTarget_free(req.target_);

        ACaptureSessionOutputContainer_remove(outputContainer, req.sessionOutput_);
        ACaptureSessionOutput_free(req.sessionOutput_);

        ANativeWindow_release(req.outputNativeWindow_);
    }

    requests_.resize(0);
    ACaptureSessionOutputContainer_free(outputContainer);

    for (auto& cam : cameras_) {
        if (cam.second.device_) {
            ACameraDevice_close(cam.second.device_);
        }
    }
    cameras_.clear();
    if (aCameraManager) {
        ACameraManager_unregisterAvailabilityCallback(aCameraManager, GetManagerListener());
        ACameraManager_delete(aCameraManager);
        aCameraManager = nullptr;
    }
}

void OnDeviceStateChanges(void* ctx, ACameraDevice* dev) {
    reinterpret_cast<cameraManager*>(ctx)->onDeviceState(dev);
}

void OnDeviceErrorChanges(void* ctx, ACameraDevice* dev, int err) {
    reinterpret_cast<cameraManager*>(ctx)->onDeviceError(dev, err);
}

ACameraDevice_stateCallbacks* cameraManager::GetDeviceListener() {
    static ACameraDevice_stateCallbacks cameraDeviceListener = {
            .context = this,
            .onDisconnected = ::OnDeviceStateChanges,
            .onError = ::OnDeviceErrorChanges,
    };
    return &cameraDeviceListener;
}

void cameraManager::onDeviceState(ACameraDevice* device) {
    std::string id(ACameraDevice_getId(device));
    LOGW("device %s is disconnected", id.c_str());

    cameras_[id].available_ = false;
    ACameraDevice_close(cameras_[id].device_);
    cameras_.erase(id);
}

void cameraManager::onDeviceError(ACameraDevice* device, int error){
    std::string id(ACameraDevice_getId(device));

    LOGI("CameraDevice %s is in error %#x", id.c_str(), error);

    CameraId& cam = cameras_[id];

    switch (error) {
        case ERROR_CAMERA_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        case ERROR_CAMERA_SERVICE:
        case ERROR_CAMERA_DEVICE:
        case ERROR_CAMERA_DISABLED:
        case ERROR_MAX_CAMERAS_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        default:
            LOGI("Unknown Camera Device Error: %#x", error);
    }
}

void OnSessionClosed(void* ctx, ACameraCaptureSession* ses) {
    LOGW("session %p closed", ses);
    reinterpret_cast<cameraManager*>(ctx)
            ->onSessionState(ses, CaptureSessionState::CLOSED);
}
void OnSessionReady(void* ctx, ACameraCaptureSession* ses) {
    LOGW("session %p ready", ses);
    reinterpret_cast<cameraManager*>(ctx)
            ->onSessionState(ses, CaptureSessionState::READY);
}
void OnSessionActive(void* ctx, ACameraCaptureSession* ses) {
    LOGW("session %p active", ses);
    reinterpret_cast<cameraManager*>(ctx)
            ->onSessionState(ses, CaptureSessionState::ACTIVE);
}

ACameraCaptureSession_stateCallbacks* cameraManager::GetSessionListener() {
    static ACameraCaptureSession_stateCallbacks sessionListener = {
            .context = this,
            .onActive = ::OnSessionActive,
            .onReady = ::OnSessionReady,
            .onClosed = ::OnSessionClosed,
    };
    return &sessionListener;
}

void cameraManager::onSessionState(ACameraCaptureSession* ses, CaptureSessionState state){
    if (!ses || ses != captureSession_) {
        LOGW("CaptureSession is %s", (ses ? "NOT our session" : "NULL"));
        return;
    }

    if(!(state < CaptureSessionState::MAX_STATE)) {
        captureSessionState_ = state;
    }
}

void OnCameraAvailable(void* ctx, const char* id) {
    reinterpret_cast<cameraManager*>(ctx)->onCameraStatusChanged(id, true);
}
void OnCameraUnavailable(void* ctx, const char* id) {
    reinterpret_cast<cameraManager*>(ctx)->onCameraStatusChanged(id, false);
}

ACameraManager_AvailabilityCallbacks* cameraManager::GetManagerListener() {
    static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
            .context = this,
            .onCameraAvailable = ::OnCameraAvailable,
            .onCameraUnavailable = ::OnCameraUnavailable,
    };
    return &cameraMgrListener;
}

void cameraManager::onCameraStatusChanged(const char* id, bool available) {
    if (valid_) {
        cameras_[std::string(id)].available_ = available ? true : false;
    }
}

void cameraManager::startPreview(bool start) {
    if (start) {
        ACameraCaptureSession_setRepeatingRequest(captureSession_, nullptr, 1,
                                         &requests_[PREVIEW_REQUEST_IDX].request_,
                                         nullptr);
    } else if (!start && captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }
}

// Capture callbacks, mostly information purpose
void SessionCaptureCallback_OnFailed(void* context,
                                     ACameraCaptureSession* session,
                                     ACaptureRequest* request,
                                     ACameraCaptureFailure* failure) {
    std::thread captureFailedThread(&cameraManager::OnCaptureFailed,
                                    static_cast<cameraManager*>(context), session,
                                    request, failure);
    captureFailedThread.detach();
}

void SessionCaptureCallback_OnSequenceEnd(void* context,
                                          ACameraCaptureSession* session,
                                          int sequenceId, int64_t frameNumber) {
    std::thread sequenceThread(&cameraManager::OnCaptureSequenceEnd,
                               static_cast<cameraManager*>(context), session,
                               sequenceId, frameNumber);
    sequenceThread.detach();
}
void SessionCaptureCallback_OnSequenceAborted(void* context,
                                              ACameraCaptureSession* session,
                                              int sequenceId) {
    std::thread sequenceThread(&cameraManager::OnCaptureSequenceEnd,
                               static_cast<cameraManager*>(context), session,
                               sequenceId, static_cast<int64_t>(-1));
    sequenceThread.detach();
}

ACameraCaptureSession_captureCallbacks* cameraManager::GetCaptureCallback() {
    static ACameraCaptureSession_captureCallbacks captureListener{
            .context = this,
            .onCaptureStarted = nullptr,
            .onCaptureProgressed = nullptr,
            .onCaptureCompleted = nullptr,
            .onCaptureFailed = SessionCaptureCallback_OnFailed,
            .onCaptureSequenceCompleted = SessionCaptureCallback_OnSequenceEnd,
            .onCaptureSequenceAborted = SessionCaptureCallback_OnSequenceAborted,
            .onCaptureBufferLost = nullptr,
    };
    return &captureListener;
}

void cameraManager::OnCaptureFailed(ACameraCaptureSession* session,
                                ACaptureRequest* request,
                                ACameraCaptureFailure* failure) {
    if (valid_ && request == requests_[JPG_CAPTURE_REQUEST_IDX].request_) {

    }
}

/**
 * Process event from JPEG capture
 *    SessionCaptureCallback_OnSequenceEnd()
 *    SessionCaptureCallback_OnSequenceAborted()
 *
 * If this is jpg capture, turn back on preview after a capture.
 */
void cameraManager::OnCaptureSequenceEnd(ACameraCaptureSession* session,
                                     int sequenceId, int64_t frameNumber) {
    if (sequenceId != requests_[JPG_CAPTURE_REQUEST_IDX].sessionSequenceId_)
        return;

    // resume preview
    ACameraCaptureSession_setRepeatingRequest(captureSession_, nullptr, 1,
                                     &requests_[PREVIEW_REQUEST_IDX].request_,
                                     nullptr);
}

// Photo taken
void cameraManager::TakePhoto() {
    if (captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }

    ACameraCaptureSession_capture(captureSession_, GetCaptureCallback(), 1,
                         &requests_[JPG_CAPTURE_REQUEST_IDX].request_,
                         &requests_[JPG_CAPTURE_REQUEST_IDX].sessionSequenceId_);
}


