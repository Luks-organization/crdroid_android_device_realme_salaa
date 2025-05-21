#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.file import File
from extract_utils.fixups_blob import (
    BlobFixupCtx,
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove,
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)
from extract_utils.tools import (
    llvm_objdump_path,
)
from extract_utils.utils import (
    run_cmd,
)

namespace_imports = [
	'device/realme/salaa',
	'hardware/mediatek',
	'hardware/mediatek/libmtkperf_client',
	'hardware/oplus',
]

def lib_fixup_odm_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'odm' else None

def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None

lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'vendor.oplus.hardware.biometrics.fingerprint@2.1',
    ): lib_fixup_odm_suffix,
    (
        'vendor.mediatek.hardware.lbs@1.0',
        'vendor.oplus.hardware.performance@1.0',
        'vendor.mediatek.hardware.videotelephony@1.0',
        'vendor.oplus.hardware.commondcs@1.0',
        'libremosaiclib',
        'libremosaic_wrapper',
        'libhwm-oplus',
    ): lib_fixup_vendor_suffix,
}

blob_fixups: blob_fixups_user_type = {
    'vendor/bin/hw/android.hardware.media.c2@1.2-mediatek-64b': blob_fixup()
       .replace_needed('libavservices_minijail_vendor.so', 'libavservices_minijail.so')
       .add_needed('libstagefright_foundation-v33.so'),
    'vendor/lib64/hw/android.hardware.camera.provider@2.6-impl-mediatek.so': blob_fixup()
       .replace_needed('libutils.so', 'libutils-v32.so')
       .add_needed('libcamera_metadata_shim.so'),
    'vendor/lib64/libmtkcam_featurepolicy.so': blob_fixup()
       .binary_regex_replace(b'\x34\xE8\x87\x40\xB9', b'\x34\x28\x02\x80\x52'),
    (
     'vendor/lib/hw/vendor.mediatek.hardware.pq@2.13-impl.so',
     'vendor/lib64/hw/vendor.mediatek.hardware.pq@2.13-impl.so',
     'vendor/lib64/libmtkcam_stdutils.so'
     ): blob_fixup()
       .replace_needed('libutils.so', 'libutils-v32.so'),
    'vendor/lib64/hw/sensors.mt6785.so': blob_fixup()
       .add_needed('libsensors_shim.so'),
    'vendor/lib64/libmnl.so': blob_fixup()
       .add_needed('libcutils.so'),
    (
     'vendor/bin/mnld',
     'vendor/lib64/libcam.utils.sensorprovider.so',
     'vendor/lib/libaalservice.so',
     'vendor/lib64/libaalservice.so',
     'vendor/lib64/hw/android.hardware.sensors@2.X-subhal-mediatek.so'
     ): blob_fixup()
       .add_needed('android.hardware.sensors@1.0-convert-shared.so'),
    ('vendor/lib64/libSQLiteModule_VER_ALL.so', 'vendor/lib64/lib3a.flash.so'): blob_fixup()
       .patchelf_version('0_17_2')
       .add_needed('liblog.so'),
    'vendor/bin/mtk_agpsd': blob_fixup()
       .replace_needed('libcrypto.so', 'libcrypto-v33.so')
       .replace_needed('libssl.so', 'libssl-v33.so'),
    'vendor/lib/hw/audio.primary.mt6785.so': blob_fixup()
       .replace_needed('libalsautils.so', 'libalsautils-v31.so'),
    (
     'vendor/lib/libnvram.so',
     'vendor/lib64/libnvram.so',
     'vendor/lib/libsysenv.so',
     'vendor/lib64/libsysenv.so',
     'vendor/bin/hw/android.hardware.neuralnetworks@1.3-service-mtk-neuron',
     'odm/bin/hw/vendor.oplus.hardware.charger@1.0-service',
     ): blob_fixup()
       .add_needed('libbase_shim.so'),
    ('vendor/lib64/libadsprpc.so', 'vendor/lib64/libcdsprpc.so'): blob_fixup()
       .replace_needed('libstdc++.so', 'libstdc++_vendor.so'),
    (
     'odm/lib/soundfx/awinic.haptic.effect.so',
     'vendor/lib/libthha.so',
     'vendor/lib/libvcodec_oal.so',
     'vendor/lib/libmp4enc_sa.ca7.so',
     'vendor/lib/libmp4enc_xa.ca7.so'
     ): blob_fixup()
       .clear_symbol_version('__aeabi_memclr')
       .clear_symbol_version('__aeabi_memcpy')
       .clear_symbol_version('__aeabi_memset')
       .clear_symbol_version('__gnu_Unwind_Find_exidx'),
    (
     'vendor/lib/libvp8dec_sa.ca7.so',
     'vendor/lib/libvp9dec_sa.ca7.so',
     'vendor/lib/libh264enc_sa.ca7.so'
     ): blob_fixup()
       .clear_symbol_version('__aeabi_memclr')
       .clear_symbol_version('__aeabi_memclr4')
       .clear_symbol_version('__aeabi_memcpy')
       .clear_symbol_version('__aeabi_memcpy4')
       .clear_symbol_version('__aeabi_memmove')
       .clear_symbol_version('__aeabi_memset')
       .clear_symbol_version('__gnu_Unwind_Find_exidx'),
    ('odm/lib64/hw/fpsensor_fingerprint.default.so', 'odm/lib64/hw/sidefp_fingerprint.default.so'): blob_fixup()
       .replace_needed('libTeeClient.so', 'libMcClient.so'),
     'vendor/bin/factory': blob_fixup()
       .replace_needed('android.hardware.light-V1-ndk_platform.so', 'android.hardware.light-V1-ndk.so'),
    'vendor/etc/vintf/manifest/manifest_media_c2_V1_1_default.xml': blob_fixup()
       .regex_replace('1.1', '1.2')
       .regex_replace('@1.0', '@1.2'),
    'vendor/etc/init/android.hardware.media.c2@1.2-mediatek.rc': blob_fixup()
       .regex_replace('@1.2-mediatek', '@1.2-mediatek-64b'),
    'vendor/bin/aee_aedv': blob_fixup()
       .add_needed('aedv_shim.so'),
    'vendor/bin/aee_aedv64': blob_fixup()
       .add_needed('aedv64_shim.so'),
    'system_ext/bin/kpoc_charger': blob_fixup()
       .add_needed('libbinder_shim.so'),
    'vendor/bin/hw/camerahalserver': blob_fixup()
       .replace_needed('libutils.so', 'libutils-v31.so'),
    'vendor/lib64/libsingle_camera_bokeh_native.so': blob_fixup()
       .clear_symbol_version('AHardwareBuffer_allocate')
       .clear_symbol_version('AHardwareBuffer_describe')
       .clear_symbol_version('AHardwareBuffer_lock')
       .clear_symbol_version('AHardwareBuffer_release')
       .clear_symbol_version('AHardwareBuffer_unlock'),
}  # fmt: skip

module = ExtractUtilsModule(
    'salaa',
    'realme',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    add_firmware_proprietary_file=True,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
