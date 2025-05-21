#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/non_ab_device.mk)

# Inherit from device makefile.
$(call inherit-product, $(LOCAL_PATH)/device.mk)

# Inherit some common crDroid OS stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# ViperFX
$(call inherit-product-if-exists, vendor/ViperFX/ViperFX.mk)

# Priv Keys
-include vendor/lineage-priv/keys/keys.mk

# AAPT
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# Boot animation
TARGET_SCREEN_HEIGHT := 2400
TARGET_SCREEN_WIDTH := 1080
TARGET_BOOT_ANIMATION_RES := 1080

# crDroid Flags
TARGET_ENABLE_BLUR := true
TARGET_DISABLE_MATLOG := true
TARGET_SUPPORTS_QUICK_TAP := true
TARGET_FACE_UNLOCK_SUPPORTED := true
TARGET_BUILD_DEVICE_AS_WEBCAM := false

# Device Information
PRODUCT_DEVICE := salaa
PRODUCT_NAME := lineage_$(PRODUCT_DEVICE)
PRODUCT_BRAND := realme
PRODUCT_MANUFACTURER := $(PRODUCT_BRAND)
PRODUCT_MODEL := RMX2151/RMX2155/RMX2156

PRODUCT_GMS_CLIENTID_BASE := android-$(PRODUCT_BRAND)

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="sys_mssi_64_cn_armv82-user 12 SP1A.210812.016 1678263634610 release-keys" \
    BuildFingerprint=realme/RMX2156/RMX2156L1:12/SP1A.210812.016/Q.174ebd4_fa4d:user/release-keys \
    DeviceProduct=$(PRODUCT_DEVICE)
