#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

import common

def FullOTA_InstallBegin(info):
  data = info.input_zip.read("RADIO/dynamic-remove-oppo")
  common.ZipWriteStr(info.output_zip, "dynamic-remove-oppo", data)
  info.script.AppendExtra('update_dynamic_partitions(package_extract_file("dynamic-remove-oppo"));')
  info.script.AppendExtra("ifelse(is_mounted(\"/apex\"), unmount(\"/apex\"));")
  return

def FullOTA_InstallEnd(info):
  OTA_InstallEnd(info, False)

def IncrementalOTA_InstallEnd(info):
  OTA_InstallEnd(info, True)

def AddImageOnly(info, basename, incremental, firmware):
  if incremental:
    input_zip = info.source_zip
  else:
    input_zip = info.input_zip
  if firmware:
    data = input_zip.read("RADIO/" + basename)
  else:
    data = input_zip.read("IMAGES/" + basename)
  common.ZipWriteStr(info.output_zip, basename, data)

def AddImage(info, basename, dest, incremental):
  AddImageOnly(info, basename, incremental, False)
  info.script.Print("Patching {} image unconditionally...".format(dest.split('/')[-1]))
  info.script.AppendExtra('package_extract_file("%s", "%s");' % (basename, dest))

def OTA_InstallEnd(info, incremental):
  AddImage(info, "dtbo.img", "/dev/block/by-name/dtbo", incremental)
  AddImage(info, "vbmeta.img", "/dev/block/by-name/vbmeta", incremental)
  AddImage(info, "vbmeta_system.img", "/dev/block/by-name/vbmeta_system", incremental)
  AddImage(info, "vbmeta_vendor.img", "/dev/block/by-name/vbmeta_vendor", incremental)

  bin_map = {
      'logo': ['logo']
  }

  img_map = {
      'audio_dsp': ['audio_dsp'],
      'cam_vpu1': ['cam_vpu1'],
      'cam_vpu2': ['cam_vpu2'],
      'cam_vpu3': ['cam_vpu3'],
      'gz': ['gz1', 'gz2'],
      'lk': ['lk', 'lk2'],
      'md1img': ['md1img'],
      'scp': ['scp1', 'scp2'],
      'spmfw': ['spmfw'],
      'sspm': ['sspm_1', 'sspm_2'],
      'tee': ['tee1', 'tee2']
  }

  pl = 'preloader_ufs'
  pl_part = ['sda', 'sdb']

  fw_cmd = 'ui_print("Patching radio images unconditionally...");\n'

  AddImageOnly(info, "{}.img".format(pl), incremental, True)
  for part in pl_part:
      fw_cmd += 'package_extract_file("{}.img", "/dev/block/{}");\n'.format(pl, part)

  for img in img_map.keys():
    AddImageOnly(info, '{}.img'.format(img), incremental, True)
    for part in img_map[img]:
      fw_cmd += 'package_extract_file("{}.img", "/dev/block/platform/bootdevice/by-name/{}");\n'.format(img, part)

  for _bin in bin_map.keys():
    AddImageOnly(info, '{}.bin'.format(_bin), incremental, True)
    for part in bin_map[_bin]:
      fw_cmd += 'package_extract_file("{}.bin", "/dev/block/platform/bootdevice/by-name/{}");\n'.format(_bin, part)

  info.script.AppendExtra(fw_cmd)
