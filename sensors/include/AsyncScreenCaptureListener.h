/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/gui/BnScreenCaptureListener.h>
#include <gui/SurfaceComposerClient.h>

namespace android {

using gui::ScreenCaptureResults;

typedef std::function<void(const ScreenCaptureResults& captureResults)> CaptureCallback;

struct AsyncScreenCaptureListener : gui::BnScreenCaptureListener {
public:
    AsyncScreenCaptureListener(CaptureCallback callback, int timeout)
            : callback(callback), timeout(timeout) {}

    binder::Status onScreenCaptureCompleted(const ScreenCaptureResults& captureResults) override {
	if (captureResults.fenceResult.ok()) { 
	if(captureResults.fenceResult.value()->wait(timeout) == OK) {
                callback(captureResults);
	    }
	return binder::Status::ok();
        }

private:
    CaptureCallback callback;
    int timeout;
};

}  // namespace android
