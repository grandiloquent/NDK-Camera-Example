//
// Created by rostislav on 27.02.19.
//

#include <string>
#include <functional>
#include <thread>
#include <cstdlib>
#include <dirent.h>
#include <ctime>
#include <vector>
#include <sys/param.h>
#include "imageReader.h"
#include "utils.h"

static const char *kDirName = "/sdcard/DCIM/Camera/";
static const char *kFileName = "capture";

void OnImageCallback(void *ctx, AImageReader *reader) {
    reinterpret_cast<imageReader *>(ctx)->imageCallback(reader);
}

imageReader::imageReader(ImageFormat *imageFormat, AIMAGE_FORMATS format) {
    callback_ = nullptr;
    callbackCtx_ = nullptr;

    media_status_t status = AImageReader_newWithUsage(imageFormat->width, imageFormat->height, format, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, 4, &reader_);

    if(status == AMEDIA_OK) {
        AImageReader_ImageListener listener{.context = this, .onImageAvailable = OnImageCallback};
        AImageReader_setImageListener(reader_, &listener);
    } else{
        LOGW("Error create listener");
    }
}

void imageReader::registerCallback(void* ctx,
                                   std::function<void(void* ctx, const char*fileName)> func) {
    callbackCtx_ = ctx;
    callback_ = func;
}

void imageReader::imageCallback(AImageReader *reader) {
    int32_t format;

    media_status_t status = AImageReader_getFormat(reader, &format);
    if (format == AIMAGE_FORMAT_YUV_420_888 && isCapture) {
        AImage *image = nullptr;
        status = AImageReader_acquireNextImage(reader, &image);


        // Create a thread and write out the jpeg files
        std::thread writeFileHandler(&imageReader::WriteFile, this, image, format);
        writeFileHandler.detach();
    }
}

ANativeWindow *imageReader::getNativeWindow(void) {
    if (!reader_) return nullptr;
    ANativeWindow *nativeWindow;
    media_status_t status = AImageReader_getWindow(reader_, &nativeWindow);

    if(status == AMEDIA_OK) return nativeWindow;
    else return nullptr;
}

AImage *imageReader::getNextImage() {
    AImage *image;
    media_status_t status = AImageReader_acquireNextImage(reader_, &image);
    if (status != AMEDIA_OK) {
        return nullptr;
    }
    return image;
}

void imageReader::deleteImage(AImage *image) {
    if (image) AImage_delete(image);
}

static const int kMaxChannelValue = 262143;

static inline uint32_t YUV2RGB(int nY, int nU, int nV) {
    nY -= 16;
    nU -= 128;
    nV -= 128;
    if (nY < 0) nY = 0;

    // This is the floating point equivalent. We do the conversion in integer
    // because some Android devices do not have floating point in hardware.
    // nR = (int)(1.164 * nY + 1.596 * nV);
    // nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
    // nB = (int)(1.164 * nY + 2.018 * nU);

    int nR = (int)(1192 * nY + 1634 * nV);
    int nG = (int)(1192 * nY - 833 * nV - 400 * nU);
    int nB = (int)(1192 * nY + 2066 * nU);

    nR = MIN(kMaxChannelValue, MAX(0, nR));
    nG = MIN(kMaxChannelValue, MAX(0, nG));
    nB = MIN(kMaxChannelValue, MAX(0, nB));

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 255 << 24 | (nB << 16) | (nG << 8) | nR ;
}

bool imageReader::displayImage(AImage *image) {

    int32_t srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    int32_t srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);

    PresentImage90(image);

    AImage_delete(image);

    return true;
}

void imageReader::PresentImage90(AImage *image) {
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);
    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);

    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    int32_t height = (srcRect.bottom - srcRect.top);
    int32_t width = (srcRect.right - srcRect.left);

    /**
     * TODO: Paste function for processing with streaming data
     */

    jint *image_out = new jint[height*width];
    for (int32_t y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int32_t x = 0; x < width; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            // [x, y]--> [-y, x]
            image_out[(x*height)+(height-y-1)] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
    }
    imagePreview(image_out, height, width);
}

void imageReader::WriteFile(AImage* image, int32_t format) {

    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);

    /**
     * TODO: This plane for processing width photo, witch take from camera.
     */

    AImage_delete(image);
}

imageReader::~imageReader() {
    AImageReader_delete(reader_);
}