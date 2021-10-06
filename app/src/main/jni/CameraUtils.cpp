#include <android/log.h>
#include <media/NdkImage.h>
#include "CameraUtils.h"


std::string GetBackFacingCameraId(ACameraManager *cameraManager) {
    ACameraIdList *cameraIds = nullptr;
    ACameraManager_getCameraIdList(cameraManager, &cameraIds);
    std::string backId;
    __android_log_print(ANDROID_LOG_ERROR, "natvie", "found camera count %d",
                        cameraIds->numCameras);
    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];
        ACameraMetadata *metadataObj;
        ACameraManager_getCameraCharacteristics(cameraManager, id, &metadataObj);
        ACameraMetadata_const_entry lensInfo = {0};
        ACameraMetadata_getConstEntry(metadataObj, ACAMERA_LENS_FACING, &lensInfo);
        auto facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                lensInfo.data.u8[0]);
        // Found a back-facing camera?
        if (facing == ACAMERA_LENS_FACING_BACK) {
            backId = id;
            break;
        }
    }
    ACameraManager_deleteCameraIdList(cameraIds);
    return backId;
}

void PrintCameraProperties(ACameraManager *cameraManager, const char *id) {
    ACameraMetadata *metadata;
    ACameraManager_getCameraCharacteristics(cameraManager, id, &metadata);
    ACameraMetadata_const_entry entry = {0};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE, &entry);
    int64_t minExposure = entry.data.i64[0];
    int64_t maxExposure = entry.data.i64[1];
    __android_log_print(ANDROID_LOG_ERROR, "natvie",
                        "Camera Properties: Exposure min = %ld, max = %ld",
                        minExposure, maxExposure);
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE, &entry);
    int32_t minSensitivity = entry.data.i32[0];
    int32_t maxSensitivity = entry.data.i32[1];
    __android_log_print(ANDROID_LOG_ERROR, "natvie",
                        "Camera Properties: Sensitivity min = %d, max = %d",
                        minSensitivity, maxSensitivity);
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    for (int i = 0; i < entry.count; i += 4) {
        int32_t input = entry.data.i32[i + 3];
        if (input)continue;
        int32_t format = entry.data.i32[i + 0];
        if (format == AIMAGE_FORMAT_JPEG) {
            int32_t width = entry.data.i32[i + 1];
            int32_t height = entry.data.i32[i + 2];
            __android_log_print(ANDROID_LOG_ERROR, "natvie",
                                "Camera Properties: maxWidth=%d vs maxHeight=%d", width, height);
        }
    }
    // cam facing
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SENSOR_ORIENTATION, &entry);
    int32_t orientation = entry.data.i32[0];
    __android_log_print(ANDROID_LOG_ERROR, "natvie", "Camera Properties: %d", orientation);
}