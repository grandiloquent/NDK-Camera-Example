//
// Created by rostislav on 27.02.19.
//
#include <jni.h>
#include <media/NdkImageReader.h>
#include <functional>

#ifndef CAMERAENGINENDK_IMAGEREADER_H
#define CAMERAENGINENDK_IMAGEREADER_H

// Struct for image format and image resolution
struct ImageFormat {
    int32_t width;
    int32_t height;

    int32_t format;  // Format image
};

class imageReader{
public:
    imageReader(ImageFormat *imageFormat, AIMAGE_FORMATS);
    ~imageReader();

    AImage* getNextImage();
    AImage* getLatestImage();
    void deleteImage(AImage* image);
    void imageCallback(AImageReader* reader);
    bool displayImage(AImage* image);
    void registerCallback(void *ctx, std::function<void(void *ctx, const char* filename)>);
    ANativeWindow *getNativeWindow(void);
    bool isCapture;

private:
    uint32_t *image_;
    AImageReader* reader_;
    std::function<void(void *ctx, const char* fileName)> callback_;
    void *callbackCtx_;
    void PresentImage90(AImage* image);
    void WriteFile(AImage* image, int32_t format);
    void imagePreview(jint *image, jint width, jint height);

};

#endif //CAMERAENGINENDK_IMAGEREADER_H

