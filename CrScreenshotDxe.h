#define gCrScreenShotGuid { 0x02e4e4f7, 0x38d9, 0x4924, {0xa4, 0xd7, 0xec, 0x6b, 0x69, 0x84, 0x7a, 0xa3 } };
typedef struct _EFI_CR_SCREEN_SHOT_PROTOCOL  EFI_CR_SCREEN_SHOT_PROTOCOL;
typedef EFI_STATUS (EFIAPI* CrScreenshotnull)(
  void *null
);

struct _EFI_CR_SCREEN_SHOT_PROTOCOL {
  EFI_CR_SCREEN_SHOT_PROTOCOL CrScreenshotnull();
}