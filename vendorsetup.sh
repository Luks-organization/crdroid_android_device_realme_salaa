#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# mediatek codec patch from hipexscape:

if [ ! -f "frameworks/av/.mtk_codec_fix" ]; then
    curl -sL "https://raw.githubusercontent.com/hipexscape/Scripts/tsm/mediatek_codec_patch.sh" | bash
    touch frameworks/av/.mtk_codec_fix
fi

# Cloning required repositories for making rum:

# Kernel Source:
git clone https://github.com/Luks-organization/kernel_realme_salaa kernel/realme/salaa

# Vendor Source:
git clone https://github.com/Luks-organization/vendor_realme_salaa vendor/realme/salaa
unzip vendor/realme/salaa/radio/md1img.zip -d vendor/realme/salaa/radio && rm vendor/realme/salaa/radio/md1img.zip
unzip vendor/realme/salaa/proprietary/odm/lib64/libstfaceunlockppl.zip -d vendor/realme/salaa/proprietary/odm/lib64 && rm vendor/realme/salaa/proprietary/odm/lib64/libstfaceunlockppl.zip

# Hardware Oplus:
git clone https://github.com/Luks-organization/Lineage_android_hardware_oplus hardware/oplus

# Hardware Mediatek:
git clone https://github.com/Luks-organization/android_hardware_mediatek hardware/mediatek

# SEPolicy:
git clone https://github.com/Luks-organization/device-mediatek-sepolicy_vndr device/mediatek/sepolicy_vndr

# DspVolumeSynchronizer:
git clone https://github.com/Luks-organization/DSPVolumeSynchronizer packages/apps/DSPVolumeSynchronizer

# RealmeParts:
git clone https://github.com/Luks-organization/RealmeParts packages/apps/RealmeParts

# ViperFX:
git clone https://github.com/Luks-organization/ViperFX vendor/ViperFX

# Apply vndk patch
#cd device/realme/salaa/patches && ./apply.sh && cd && cd evo

# CameraGo:
#git clone https://github.com/Luks-organization/CameraGo vendor/CameraGo

# CameraGo:
#unzip vendor/CameraGo/CameraGo/CameraGo.zip -d vendor/CameraGo/CameraGo && rm vendor/CameraGo/CameraGo/CameraGo.zip

# OplusDolby:
#git clone https://github.com/Luks-organization/DolbyManager packages/apps/OplusDolby

# Clang:
#git clone --depth=1 https://gitlab.com/RismaPwd/clang toolchain/clang-proton

# Make the build faster using ccache
export USE_CCACHE=1
export CCACHE_DIR=~/.ccache
ccache -M 40G
ccache -o compression=true

# Disable and stop systemd-oomd service.
systemctl disable --now systemd-oomd && sudo apt-get purge systemd-oomd -y
