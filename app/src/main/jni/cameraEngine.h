//
// Created by rostislav on 27.02.19.
//

#import "utils.h"
#import "imageReader.h"
#import "cameraManager.h"

#ifndef CAMERAENGINENDK_CAMERAENGINE_H
#define CAMERAENGINENDK_CAMERAENGINE_H

class cameraEngine{
public:
    cameraEngine(DisplayDimension *Dim, bool isCameraBack);
    ~cameraEngine();

    void createCamera();
    void deleteCamera();
    void refrashFacing();

    void drawFrame();
    void startPreview(bool start);
    void onTakeImage();

private:
    imageReader* previewReader; //Preview reader
    imageReader* captureReader; //Capture reader

    cameraManager* cameraMgr; // Main camera manager
    DisplayDimension *size_operation;
    uint32_t facing = 1;
    bool CameraReady;

};

#endif //CAMERAENGINENDK_CAMERAENGINE_H
