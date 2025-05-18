/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <cstdlib>
#include <fstream>
#include <cstring>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "vendor_init.h"
#include "property_service.h"

#include <fs_mgr_dm_linear.h>

using android::base::ReadFileToString;

const char* const OPERATOR_CODE_FILE = "/proc/oplusVersion/operatorName";

void property_override(char const prop[], char const value[], bool add = true)
{
    prop_info *pi;

    pi = (prop_info *) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void set_ro_build_prop(const std::string& prop, const std::string& value,
                       bool product = true) {
    std::string prop_name;
    std::vector<std::string> prop_types = {
            "",        "bootimage.",  "odm.",    "odm_dlkm.",   "product.",
            "system.", "system_dlkm.", "system_ext.", "vendor.", "vendor_dlkm."};

    for (const auto& source : prop_types) {
        if (product) {
            prop_name = "ro.product." + source + prop;
        } else {
            prop_name = "ro." + source + "build." + prop;
        }

        property_override(prop_name.c_str(), value.c_str(), false);
    }
}

// Function to set Dalvik VM properties based on total RAM size
void load_dalvik_properties(void)
{
    char const *heapstartsize;
    char const *heapgrowthlimit;
    char const *heapsize;
    char const *heapminfree;
    char const *heapmaxfree;
    char const *heaptargetutilization;
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram >= 7ull * 1024 * 1024 * 1024) {
        // from - phone-xhdpi-8192-dalvik-heap.mk
        heapstartsize = "24m";
        heapgrowthlimit = "384m";
        heapsize = "512m";
        heaptargetutilization = "0.46";
        heapminfree = "8m";
        heapmaxfree = "48m";
    } else if (sys.totalram >= 5ull * 1024 * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else {
        // from - phone-xhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    }

    // Apply heap properties
    property_override("dalvik.vm.heapstartsize", heapstartsize);
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_override("dalvik.vm.heapsize", heapsize);
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", heapminfree);
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);
}

// Function to set device-specific properties based on operator code
void set_device_props(void) {
    std::string operator_code_raw, model, device, marketname, fingerprint;
    if (ReadFileToString(OPERATOR_CODE_FILE, &operator_code_raw)) {
        int operator_code = std::stoi(operator_code_raw);
        // Mapping operator codes to device properties
        switch (operator_code) {
            case 140:
            case 141:
            case 146:
            case 149:
                device="RMX2151L1";
                model="RMX2151";
                fingerprint="realme/RMX2151/RMX2151L1:12/SP1A.210812.016/Q.bf75e7-1:user/release-keys";
                marketname="realme 7";
                break;
            case 94:
            case 148:
                device="RMX2155L1";
                model="RMX2155";
                fingerprint="realme/RMX2155EEA/RMX2155L1:12/SP1A.210812.016/Q.GDPR.bf75e7-1:user/release-keys";
                marketname="realme 7";
                break;
            case 90:
            case 92:
                device="RMX2156L1";
                model="RMX2156";
                fingerprint="realme/RMX2156/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys";
                marketname="realme Narzo 30 4G";
                break;
            default:
                LOG(ERROR) << "Unknown operator found: " << operator_code;
                device="";
                model="";
                fingerprint="";
                marketname="";
		}
    }

    // Set build properties
    set_ro_build_prop("model", model);
    set_ro_build_prop("device", device);
    set_ro_build_prop("name", model);
    set_ro_build_prop("product", model, false);
    set_ro_build_prop("marketname", marketname);
    set_ro_build_prop("fingerprint", fingerprint);

    // Additional properties
    property_override("ro.product.name", model.c_str());
    property_override("ro.product.device", device.c_str());
    property_override("ro.vendor.device", device.c_str());
    property_override("ro.product.marketname", marketname.c_str());
    property_override("ro.oplus.market.name", marketname.c_str());
    property_override("ro.vendor.oplus.market.name", marketname.c_str());
    property_override("bluetooth.device.default_name", marketname.c_str());
    property_override("vendor.usb.product_string", marketname.c_str());
    property_override("ro.build.fingerprint", fingerprint.c_str());
    property_override("ro.product.build.fingerprint", fingerprint.c_str());
    property_override("ro.system.build.fingerprint", fingerprint.c_str());
    property_override("ro.system_dlkm.build.fingerprint", fingerprint.c_str());
    property_override("ro.vendor.build.fingerprint", fingerprint.c_str());
    property_override("ro.vendor_dlkm.build.fingerprint", fingerprint.c_str());
    property_override("ro.odm.build.fingerprint", fingerprint.c_str());
    property_override("ro.odm_dlkm.build.fingerprint", fingerprint.c_str());
    property_override("ro.system_ext.build.fingerprint", fingerprint.c_str());
    property_override("ro.bootimage.build.fingerprint", fingerprint.c_str());
}

void vendor_load_properties(void) {
#ifndef __ANDROID_RECOVERY__
    set_device_props();
#endif
    load_dalvik_properties();
}

