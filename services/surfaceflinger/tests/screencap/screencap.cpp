/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <SkImageEncoder.h>
#include <SkBitmap.h>

using namespace android;

static SkBitmap::Config flinger2skia(PixelFormat f)
{
    switch (f) {
        case PIXEL_FORMAT_RGB_565:
            return SkBitmap::kRGB_565_Config;
        default:
            return SkBitmap::kARGB_8888_Config;
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("usage: %s path\n", argv[0]);
        exit(0);
    }

    void const* base = 0;
    uint32_t w, s, h, f;
    size_t size = 0;

    ScreenshotClient screenshot;
    const String16 name("SurfaceFlinger");
    sp<ISurfaceComposer> composer;
    getService(name, &composer);
    sp<IBinder> display(composer->getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
    if (display != NULL && screenshot.update(display) == NO_ERROR) {
        base = screenshot.getPixels();
        w = screenshot.getWidth();
        h = screenshot.getHeight();
        s = screenshot.getStride();
        f = screenshot.getFormat();
        size = screenshot.getSize();
    }

    printf("screen capture success: w=%u, h=%u, pixels=%p\n",
            w, h, base);

    printf("saving file as PNG in %s ...\n", argv[1]);

    SkBitmap b;
    b.setConfig(flinger2skia(f), w, h, s*bytesPerPixel(f));
    b.setPixels((void*)base);
    SkImageEncoder::EncodeFile(argv[1], b,
            SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);

    return 0;
}
