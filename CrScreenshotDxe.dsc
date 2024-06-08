################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = DcsPkg
  PLATFORM_GUID                  = 5a9e7754-d81b-49ea-85ad-69eaa7b1539b
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/DcsPkg
  SUPPORTED_ARCHITECTURES        = X64 | IA32
  BUILD_TARGETS                  = DEBUG|RELEASE

[BuildOptions]
  GCC:*_UNIXGCC_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  GCC:RELEASE_*_*_CC_FLAGS             = -DMDEPKG_NDEBUG
  INTEL:RELEASE_*_*_CC_FLAGS           = /D MDEPKG_NDEBUG# /Zc:wchar_t- /wd4066 /wd4458 /wd4804 /Od
  MSFT:RELEASE_*_*_CC_FLAGS            = /D MDEPKG_NDEBUG# /Zc:wchar_t- /wd4066 /wd4458 /wd4804 /Od
  GCC:*_*_*_CC_FLAGS                   = -mno-mmx -mno-sse


################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  CrScreenshotDxeLib|CrScreenshotDxe.inf
  
################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform.
#
################################################################################
[Components]
  CrScreenshotDxe.inf
