//
// Created by rostislav on 27.02.19.
//

#include "cameraEngine.h"
#import <jni.h>
#import <cstdio>
#include <cstring>

// constructor class of camera engine
//@param: dimension in class DisplayDimension
cameraEngine::cameraEngine(DisplayDimension *Dim, bool isCameraBack) :
        captureReader(nullptr),
        previewReader(nullptr) {
    size_operation = Dim;
    LOGI("Camera created successful");
    if (isCameraBack) facing = 1;
    else facing = 0;
    cameraMgr = new cameraManager(facing);
    createCamera();
}

// Function for camera create and open it
void cameraEngine::createCamera() {
    cameraMgr->openCamera();

    ImageFormat view{0, 0, 0}, capture{0, 0, 0}; // Create image resolution
    cameraMgr->maxCaptureSize(&view, &capture,
                              size_operation); // take maximum image resolution for this camera and resolution for preview

    //Create readers
    previewReader = new imageReader(&view, AIMAGE_FORMAT_YUV_420_888);
    captureReader = new imageReader(&capture, AIMAGE_FORMAT_JPEG);
    //captureReader = new imageReader(&capture, AIMAGE_FORMAT_YUV_420_888);
    captureReader->isCapture = true;

    cameraMgr->CreateSession(previewReader->getNativeWindow(), captureReader->getNativeWindow(),
                             90);
}

//Function for delete cameraEngine
void cameraEngine::deleteCamera(void) {
    CameraReady = false;
    if (cameraMgr) {
        delete cameraMgr;
        cameraMgr = nullptr;
    }
    if (previewReader) {
        delete previewReader;
        previewReader = nullptr;
    }
    if (captureReader) {
        delete captureReader;
        captureReader = nullptr;
    }
}

// Draw frame to ImageView
// In the moment load next image in buffer
void cameraEngine::drawFrame() {
    //if(!CameraReady || !previewReader) return nullptr;
    AImage *image = previewReader->getNextImage();

    if (!image) return;
    previewReader->displayImage(image);
}

// Start Preview.
// For run Thread
void cameraEngine::startPreview(bool start) {
    if (start == true) {
        cameraMgr->startPreview(true);
    }
}

// Take Image
void cameraEngine::onTakeImage() {
    if (cameraMgr) {
        cameraMgr->TakePhoto();
    }
}