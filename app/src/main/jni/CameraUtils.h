#ifndef CAMERAUTILS_H
#define CAMERAUTILS_H
// #include "CameraUtils.h"
#include <string>
#include <camera/NdkCameraManager.h>


std::string GetBackFacingCameraId(ACameraManager *cameraManager);

void PrintCameraProperties(ACameraManager *cameraManager, const char *id);

#endif
