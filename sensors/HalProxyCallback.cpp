/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "HalProxyCallback.h"

#include <cinttypes>

namespace android {
namespace hardware {
namespace sensors {
namespace V2_0 {
namespace implementation {

static constexpr int32_t kBitsAfterSubHalIndex = 24;

/**
 * Set the subhal index as first byte of sensor handle and return this modified version.
 *
 * @param sensorHandle The sensor handle to modify.
 * @param subHalIndex The index in the hal proxy of the sub hal this sensor belongs to.
 *
 * @return The modified sensor handle.
 */
int32_t setSubHalIndex(int32_t sensorHandle, size_t subHalIndex) {
    return sensorHandle | (static_cast<int32_t>(subHalIndex) << kBitsAfterSubHalIndex);
}

void HalProxyCallbackBase::postEvents(const std::vector<V2_1::Event>& events,
                                      ScopedWakelock wakelock) {
    if (events.empty() || !mCallback->areThreadsRunning()) return;
    size_t numWakeupEvents;
    std::vector<V2_1::Event> processedEvents = processEvents(events, &numWakeupEvents);
    if (numWakeupEvents > 0) {
        ALOG_ASSERT(wakelock.isLocked(),
                    "Wakeup events posted while wakelock unlocked for subhal"
                    " w/ index %" PRId32 ".",
                    mSubHalIndex);
    } else {
        ALOG_ASSERT(!wakelock.isLocked(),
                    "No Wakeup events posted but wakelock locked for subhal"
                    " w/ index %" PRId32 ".",
                    mSubHalIndex);
    }
    mCallback->postEventsToMessageQueue(processedEvents, numWakeupEvents, std::move(wakelock));
}

ScopedWakelock HalProxyCallbackBase::createScopedWakelock(bool lock) {
    ScopedWakelock wakelock(mRefCounter, lock);
    return wakelock;
}

std::vector<V2_1::Event> HalProxyCallbackBase::processEvents(const std::vector<V2_1::Event>& events,
                                                             size_t* numWakeupEvents) const {
    *numWakeupEvents = 0;
    std::vector<V2_1::Event> eventsOut;
    for (V2_1::Event event : events) {
        event.sensorHandle = setSubHalIndex(event.sensorHandle, mSubHalIndex);
        if (event.sensorType == V2_1::SensorType::DYNAMIC_SENSOR_META) {
            event.u.dynamic.sensorHandle =
                    setSubHalIndex(event.u.dynamic.sensorHandle, mSubHalIndex);
        }
        eventsOut.push_back(event);
        const V2_1::SensorInfo& sensor = mCallback->getSensorInfo(event.sensorHandle);
        if ((sensor.flags & V1_0::SensorFlagBits::WAKE_UP) != 0) {
            (*numWakeupEvents)++;
        }
    }
    return eventsOut;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
