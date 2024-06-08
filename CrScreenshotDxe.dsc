## @file  MdePkg.dec
# This Package provides all definitions, library classes and libraries instances.
#
# It also provides the definitions(including PPIs/PROTOCOLs/GUIDs) of
# EFI1.10/UEFI2.7/PI1.7 and some Industry Standards.
#
# Copyright (c) 2007 - 2019, Intel Corporation. All rights reserved.<BR>
# Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
# (C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
#
# This program and the accompanying materials are licensed and made available under
# the terms and conditions of the BSD License which accompanies this distribution.
# The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##


[Defines]
  DEC_SPECIFICATION              = 0x00010005
  PACKAGE_NAME                   = MdePkg
  PACKAGE_UNI_FILE               = MdePkg.uni
  PACKAGE_GUID                   = 1E73767F-8F52-4603-AEB4-F29B510B6766
  PACKAGE_VERSION                = 1.08


[Includes]
  Include

[Includes.IA32]
  Include/Ia32

[Includes.X64]
  Include/X64

[Includes.EBC]
  Include/Ebc

[Includes.ARM]
  Include/Arm

[Includes.AARCH64]
  Include/AArch64

[LibraryClasses]
  ##  @libraryclass  Provides most usb APIs to support the Hid requests defined in Usb Hid 1.1 spec
  #                  and the standard requests defined in Usb 1.1 spec.
  ##
  UefiUsbLib|Include/Library/UefiUsbLib.h
  CrScreenshotDxeLib|CrScreenshotDxe.h

[Guids]
  gEfiPlatformToDriverConfigurationClpGuid = { 0x345ecc0e, 0xcb6, 0x4b75, { 0xbb, 0x57, 0x1b, 0x12, 0x9c, 0x47, 0x33,0x3e }}
  gEfiCrscreenshotDxeGuid = { 0x02e4e4f7, 0x38d9, 0x4924, {0xa4, 0xd7, 0xec, 0x6b, 0x69, 0x84, 0x7a, 0xa3 } }