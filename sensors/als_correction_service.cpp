/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/properties.h>
#include <binder/ProcessState.h>
#include <gui/LayerState.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/SyncScreenCaptureListener.h>
#include <ui/DisplayState.h>

#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <sysutils/FrameworkCommand.h>
#include <sysutils/FrameworkListener.h>
#include <utils/Timers.h>

using android::base::SetProperty;
using android::ui::Rotation;
using android::ui::DisplayState;
using android::GraphicBuffer;
using android::Rect;
using android::ScreenshotClient;
using android::sp;
using android::SurfaceComposerClient;
using namespace android;

constexpr int ALS_POS_X = 610;
constexpr int ALS_POS_Y = 40;
constexpr int ALS_RADIUS = 64;

static sp<IBinder> getInternalDisplayToken() {
	const auto displayIds = SurfaceComposerClient::getPhysicalDisplayIds();
	sp<IBinder> token = SurfaceComposerClient::getPhysicalDisplayToken(displayIds[0]);
	return token;
}

static Rect screenshot_rect_0(ALS_POS_X - ALS_RADIUS, ALS_POS_Y - ALS_RADIUS, ALS_POS_X + ALS_RADIUS, ALS_POS_Y + ALS_RADIUS);
static Rect screenshot_rect_land_90(ALS_POS_Y - ALS_RADIUS, 1080 - ALS_POS_X - ALS_RADIUS, ALS_POS_Y + ALS_RADIUS, 1080 - ALS_POS_X + ALS_RADIUS);
static Rect screenshot_rect_180(1080-ALS_POS_X - ALS_RADIUS, 2400-ALS_POS_Y - ALS_RADIUS, 1080-ALS_POS_X + ALS_RADIUS, 2400-ALS_POS_Y + ALS_RADIUS);
static Rect screenshot_rect_land_270(2400 - (ALS_POS_Y + ALS_RADIUS),ALS_POS_X - ALS_RADIUS, 2400 - (ALS_POS_Y - ALS_RADIUS), ALS_POS_X + ALS_RADIUS);

class TakeScreenshotCommand : public FrameworkCommand {
  public:
    TakeScreenshotCommand() : FrameworkCommand("take_screenshot") {}
    ~TakeScreenshotCommand() override = default;

    int runCommand(SocketClient* cli, int /*argc*/, char **/*argv*/) {
        auto screenshot = takeScreenshot();
        cli->sendData(&screenshot, sizeof(screenshot_t));
        return 0;
    }
  private:
    struct screenshot_t {
        uint32_t r, g, b;
        nsecs_t timestamp;
    };

    screenshot_t takeScreenshot() {
        sp<IBinder> display = getInternalDisplayToken();

        android::ui::DisplayState state;
        SurfaceComposerClient::getDisplayState(display, &state);
        Rect screenshot_rect;
        switch (state.orientation) {
             case Rotation::Rotation90:  screenshot_rect = screenshot_rect_land_90;
                                         break;
             case Rotation::Rotation180: screenshot_rect = screenshot_rect_180;
                                         break;
             case Rotation::Rotation270: screenshot_rect = screenshot_rect_land_270;
                                         break;
             default:                    screenshot_rect = screenshot_rect_0;
                                         break;
        }

        sp<SyncScreenCaptureListener> captureListener = new SyncScreenCaptureListener();
        gui::ScreenCaptureResults captureResults;

        static sp<GraphicBuffer> outBuffer = new GraphicBuffer(
        screenshot_rect.getWidth(), screenshot_rect.getHeight(),
        android::PIXEL_FORMAT_RGB_888,
        GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN);

        DisplayCaptureArgs captureArgs;
        captureArgs.displayToken = display;
        captureArgs.captureArgs.pixelFormat = static_cast<int>(android::PIXEL_FORMAT_RGBA_8888);

        android::gui::ARect rect;
        captureArgs.captureArgs.sourceCrop = rect;
        captureArgs.width = screenshot_rect.getWidth();
        captureArgs.height = screenshot_rect.getHeight();
        //captureArgs.useIdentityTransform = false;
        status_t ret = ScreenshotClient::captureDisplay(captureArgs, captureListener);

        uint8_t *out;
        auto resultWidth = outBuffer->getWidth();
        auto resultHeight = outBuffer->getHeight();
        auto stride = outBuffer->getStride();

        outBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, reinterpret_cast<void **>(&out));
        // we can sum this directly on linear light
        uint32_t rsum = 0, gsum = 0, bsum = 0;
        for (int y = 0; y < resultHeight; y++) {
            for (int x = 0; x < resultWidth; x++) {
                rsum += out[y * (stride * 4) + x * 4];
                gsum += out[y * (stride * 4) + x * 4 + 1];
                bsum += out[y * (stride * 4) + x * 4 + 2];
            }
        }
        uint32_t max = resultWidth * resultHeight;
        outBuffer->unlock();

        return { rsum / max, gsum / max, bsum / max, systemTime(SYSTEM_TIME_BOOTTIME) };
    }
};

class AlsCorrectionListener : public FrameworkListener {
  public:
    AlsCorrectionListener() : FrameworkListener("als_correction") {
        registerCmd(new TakeScreenshotCommand);
    }
};

int main() {
    ProcessState::self()->setThreadPoolMaxThreadCount(0);
    ProcessState::self()->startThreadPool();

    auto listener = new AlsCorrectionListener();
    listener->startListener();

    while (true) {
        pause();
    }

    return 0;
}
