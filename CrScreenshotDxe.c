/* CrScreenshotDxe.c

Copyright (c) 2016, Nikolaj Schlej, All rights reserved.

Redistribution and use in source and binary forms, 
with or without modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, 
  this list of conditions and the following disclaimer in the documentation 
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#pragma optimize( "", off )

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/MemoryAllocationLib.h>

#include "lodepng.h" //PNG encoding library

EFI_STATUS
EFIAPI
FindWritableFs (
    OUT EFI_FILE_PROTOCOL **WritableFs
    )
{
    EFI_HANDLE *HandleBuffer = NULL;
    UINTN      HandleCount;
    UINTN      i;
    EFI_STATUS FsStatus;
    FsStatus = EFI_ABORTED;
    
    // Locate all the simple file system devices in the system
    EFI_STATUS Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (!EFI_ERROR (Status)) {
        EFI_FILE_PROTOCOL *Fs = NULL;
        // For each located volume
        for (i = 0; i < HandleCount; i++) {
            EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFs = NULL;
            EFI_FILE_PROTOCOL *File = NULL;
            
            // Get protocol pointer for current volume
            Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiSimpleFileSystemProtocolGuid, (VOID **) &SimpleFs);
            if (EFI_ERROR (Status)) {
                Print (L"FindWritableFs: gBS->HandleProtocol[%d] returned %r\n", i, Status);
                continue;
            }
            
            // Open the volume
            Status = SimpleFs->OpenVolume(SimpleFs, &Fs);
            if (EFI_ERROR (Status)) {
                Print (L"FindWritableFs: SimpleFs->OpenVolume[%d] returned %r\n", i, Status);
                continue;
            }
            
            // Try opening a file for writing
            if (*WritableFs == NULL) {
                Status = Fs->Open(Fs, &File, L"screenshot\\crsdtest.fil", EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
                if (EFI_ERROR (Status)) {
                    Print (L"FindWritableFs: Fs->Open[%d] returned %r\n", i, Status);
                    continue;
                }
                
                // Writable FS found
                *WritableFs = Fs;
                Fs->Delete(File);
                FsStatus = EFI_SUCCESS;
            } else {
                Status = Fs->Open(Fs, &File, L"screenshot\\crsdtest.fil", EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
                if (EFI_ERROR (Status)) {
                    Print (L"FindWritableFs: Fs->Open[%d] returned %r\n", i, Status);
                    continue;
                }
                
                // Writable FS found
                // *WritableFs = Fs;
                Fs->Delete(File);
                FsStatus = EFI_ABORTED;
            }
        }
    }
    
    // Free memory
    if (HandleBuffer) {
        gBS->FreePool(HandleBuffer);
    }
    
    return FsStatus;
}

EFI_STATUS
EFIAPI
ShowStatus (
    IN UINT8 Red, 
    IN UINT8 Green, 
    IN UINT8 Blue
    )
{
    // Determines the size of status square
    #define STATUS_SQUARE_SIDE 30

    UINTN        HandleCount;
    EFI_HANDLE   *HandleBuffer = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput = NULL;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Square[STATUS_SQUARE_SIDE * STATUS_SQUARE_SIDE];
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Backup[STATUS_SQUARE_SIDE * STATUS_SQUARE_SIDE];
    UINTN i;
    
    // Locate all instances of GOP
    EFI_STATUS Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR (Status)) {
        Print (L"ShowStatus: Graphics output protocol not found\n");
        return EFI_UNSUPPORTED;
    }
    
    // Set square color
    for (i = 0 ; i < STATUS_SQUARE_SIDE * STATUS_SQUARE_SIDE; i++) {
        Square[i].Blue = Blue;
        Square[i].Green = Green;
        Square[i].Red = Red;
        Square[i].Reserved = 0x00;
    }
    
    // For each GOP instance
    for (i = 0; i < HandleCount; i ++) {
        // Handle protocol
        Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
        if (EFI_ERROR (Status)) {
            Print (L"ShowStatus: gBS->HandleProtocol[%d] returned %r\n", i, Status);
            continue;
        }
            
        // Backup current image
        GraphicsOutput->Blt(GraphicsOutput, Backup, EfiBltVideoToBltBuffer, 0, 0, 0, 0, STATUS_SQUARE_SIDE, STATUS_SQUARE_SIDE, 0);
        
        // Draw the status square
        GraphicsOutput->Blt(GraphicsOutput, Square, EfiBltBufferToVideo, 0, 0, 0, 0, STATUS_SQUARE_SIDE, STATUS_SQUARE_SIDE, 0);
        
        // Wait 100ms
        gBS->Stall(100*1000);
        
        // Restore the backup
        GraphicsOutput->Blt(GraphicsOutput, Backup, EfiBltBufferToVideo, 0, 0, 0, 0, STATUS_SQUARE_SIDE, STATUS_SQUARE_SIDE, 0);
    }
    
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
TakeScreenshot (
    IN EFI_KEY_DATA *KeyData
    )
{
    EFI_FILE_PROTOCOL *Fs = NULL;
    EFI_FILE_PROTOCOL *File = NULL;
    EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput = NULL;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Image = NULL;
    UINTN      ImageSize;         // Size in pixels
    UINT8      *PngFile = NULL;
    UINTN      PngFileSize;       // Size in bytes
    EFI_STATUS Status;
    UINTN      HandleCount;
    EFI_HANDLE *HandleBuffer = NULL;
    UINT32     ScreenWidth;
    UINT32     ScreenHeight;
    CHAR16     FileName[255]; // 0-terminated 8.3 file name
    EFI_TIME   Time;
    UINTN      i, j;

    // Find writable FS
    Status = FindWritableFs(&Fs);
    if (EFI_ERROR (Status)) {
        Print (L"TakeScreenshot: Can't find writable FS\n");
        ShowStatus(0xFF, 0xFF, 0x00); //Yellow
        return EFI_SUCCESS;
    }
    
    // Locate all instances of GOP
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR (Status)) {
        Print (L"ShowStatus: Graphics output protocol not found\n");
        return EFI_SUCCESS;
    }
    
    // For each GOP instance
    for (i = 0; i < HandleCount; i++) {
        do { // Break from do used instead of "goto error"
            // Handle protocol
            Status = gBS->HandleProtocol(HandleBuffer[i], &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
            if (EFI_ERROR (Status)) {
                Print (L"ShowStatus: gBS->HandleProtocol[%d] returned %r\n", i, Status);
                break;
            }
        
            // Set screen width, height and image size in pixels
            ScreenWidth  = GraphicsOutput->Mode->Info->HorizontalResolution;
            ScreenHeight = GraphicsOutput->Mode->Info->VerticalResolution;
            ImageSize = ScreenWidth * ScreenHeight;
            
            // Get current time
            Status = gRT->GetTime(&Time, NULL);
            if (!EFI_ERROR(Status)) {
                // Set file name to current day and time
                UnicodeSPrint(FileName, 255, L"screenshot\\%02d-%02d-%02d-%02d_%02d_%02d.png", Time.Year, Time.Month, Time.Day, Time.Hour, Time.Minute, Time.Second);
            }
            else {
                // Set file name to scrnshot.png
                UnicodeSPrint(FileName, 255, L"screenshot\\scrnshot.png");
            }
            
            // Allocate memory for screenshot
            Status = gBS->AllocatePool(EfiBootServicesData, ImageSize * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (VOID **)&Image);
            if (EFI_ERROR(Status)) {
                Print (L"TakeScreenshot: gBS->AllocatePool returned %r\n", Status);
                break;
            }
        
            // Take screenshot
            Status = GraphicsOutput->Blt(GraphicsOutput, Image, EfiBltVideoToBltBuffer, 0, 0, 0, 0, ScreenWidth, ScreenHeight, 0);
            if (EFI_ERROR(Status)) {
                Print (L"TakeScreenshot: GraphicsOutput->Blt returned %r\n", Status);
                break;
            }
            
            // Check for pitch black image (it means we are using a wrong GOP)
            for (j = 0; j < ImageSize; j++) {
                if (Image[j].Red != 0x00 || Image[j].Green != 0x00 || Image[j].Blue != 0x00)
                    break;
            }
            if (j == ImageSize) {
                Print (L"TakeScreenshot: GraphicsOutput->Blt returned pitch black image, skipped\n");
                ShowStatus(0x00, 0x00, 0xFF); //Blue
                break;
            }
            
            // Open or create output file
            Status = Fs->Open(Fs, &File, FileName, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
            if (EFI_ERROR (Status)) {
                Print (L"TakeScreenshot: Fs->Open of %s returned %r\n", FileName, Status);
                break;
            }
            
            // Convert BGR to RGBA with Alpha set to 0xFF
            for (j = 0; j < ImageSize; j++) {
                UINT8 Temp = Image[j].Blue;
                Image[j].Blue = Image[j].Red;
                Image[j].Red = Temp;
                Image[j].Reserved = 0xFF;
            }
        
            // Encode raw RGB image to PNG format
            j = lodepng_encode32(&PngFile, &PngFileSize, (CONST UINT8*)Image, ScreenWidth, ScreenHeight);
            if (j) {
                Print (L"TakeScreenshot: lodepng_encode32 returned %d\n", j);
                break;
            }
                
            // Write PNG image into the file and close it
            Status = File->Write(File, &PngFileSize, PngFile);
            File->Close(File);
            if (EFI_ERROR(Status)) {
                Print (L"TakeScreenshot: File->Write returned %r\n", Status);
                break;
            }
            
            // Show success
            ShowStatus(0x00, 0xFF, 0x00); //Green
        } while(0);
        
        // Free memory
        if (Image)
            gBS->FreePool(Image);
        if (PngFile)
            lodepng_free(PngFile);
        Image = NULL;
        PngFile = NULL;
    }
    
    // Show error
    if (EFI_ERROR(Status))
        ShowStatus(0xFF, 0x00, 0x00); //Red
    
    return EFI_SUCCESS;
}

typedef struct KeyFuncBuffStruct{
    UINT16 ScanCode;
    EFI_KEY_NOTIFY_FUNCTION KeyNotificationFunction;
} KeyFuncBuff;

EFI_KEY_DATA EmptyKeyData;

void emptykeydata ()
{
    EmptyKeyData.Key.ScanCode = 0;
    EmptyKeyData.Key.UnicodeChar = 0;
    EmptyKeyData.KeyState.KeyShiftState = 0;
    EmptyKeyData.KeyState.KeyToggleState = 0;
}

void ReadKeyStroke (IN EFI_EVENT Event, IN VOID *Context)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    KeyFuncBuff *Buff = Context;
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    //Print(L"ScanCode set: %0X\n", Buff->ScanCode);
    //Print(L"ScanCode: %0X\n", Key.ScanCode);
    if (Status == EFI_SUCCESS) {
        Print(L"ScanCode set: %0X\n", Buff->ScanCode);
        if (Buff->ScanCode == Key.ScanCode) {
            (Buff->KeyNotificationFunction)(&EmptyKeyData);
        }
        Print(L"ScanCode: %0X\n", Key.ScanCode);
    }
}


EFI_STATUS EFIAPI SimpleTextInWaitForKeyStroke (
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    IN EFI_INPUT_KEY *KeyInput,
    IN EFI_KEY_NOTIFY_FUNCTION KeyNotificationFunction,
    OUT VOID **NotifyHandle
    )
{
    EFI_EVENT TimeEvent;
    EFI_STATUS Status;
    KeyFuncBuff *Buff = AllocatePool(sizeof(KeyFuncBuff));
    Buff->ScanCode = KeyInput->ScanCode;
    Buff->KeyNotificationFunction = KeyNotificationFunction;
    Print(L"ScanCode set: %0X\n", Buff->ScanCode);
    Status = gBS->CreateEvent(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, (EFI_EVENT_NOTIFY)ReadKeyStroke, Buff, &TimeEvent);
    Print(L"Status: %r\n", Status);
    if (EFI_ERROR (Status)) {
        Print (L"gBS->CreateEvent Failed: %r\n", Status);
        return Status;
    }
    Status = gBS->SetTimer(TimeEvent, TimerPeriodic, 5 * 1000 * 1000);
    if (EFI_ERROR (Status)) {
        Print (L"gBS->SetTimer Failed: %r\n", Status);
        return Status;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CrScreenshotDxeEntry (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
    )
{
    EFI_STATUS                        Status;
    UINTN                             HandleCount = 0;
    EFI_HANDLE                        *HandleBuffer = NULL;
    UINTN                             Index;
    EFI_KEY_DATA                      SimpleTextInExKeyStroke;
    EFI_KEY_DATA                      SimpleTextInExKeyStrokeLeft;
    EFI_KEY_DATA                      SimpleTextInExKeyStrokeRight;
    EFI_KEY_DATA                      SimpleTextInExKeyStrokeLeftShift;
    EFI_KEY_DATA                      SimpleTextInExKeyStrokeRightShift;
    EFI_INPUT_KEY                     SimpleTextInKeyStrokeF2;
    EFI_INPUT_KEY                     SimpleTextInKeyStrokeF4;
    EFI_INPUT_KEY                     SimpleTextInKeyStrokeF8;
    EFI_INPUT_KEY                     SimpleTextInKeyStrokeF10;
    EFI_HANDLE                        SimpleTextInExHandle;
    EFI_HANDLE                        SimpleTextInHandle;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleTextInEx;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *SimpleTextIn;
    BOOLEAN                           Installed = FALSE;
    EFI_GUID                          gEfiCrscreenshotDxeGuid;
    EFI_HANDLE                        CrScreenHandle = NULL;
    UINTN                             CrHandleCount = 0;

    gEfiCrscreenshotDxeGuid.Data1 = 0x02e4e4f7;
    gEfiCrscreenshotDxeGuid.Data2 = 0x38d9;
    gEfiCrscreenshotDxeGuid.Data3 = 0x4924;
    gEfiCrscreenshotDxeGuid.Data4[0] = 0xa4;
    gEfiCrscreenshotDxeGuid.Data4[1] = 0xd7;
    gEfiCrscreenshotDxeGuid.Data4[2] = 0xec;
    gEfiCrscreenshotDxeGuid.Data4[3] = 0x6b;
    gEfiCrscreenshotDxeGuid.Data4[4] = 0x69;
    gEfiCrscreenshotDxeGuid.Data4[5] = 0x84;
    gEfiCrscreenshotDxeGuid.Data4[6] = 0x7a;
    gEfiCrscreenshotDxeGuid.Data4[7] = 0xa3;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiCrscreenshotDxeGuid, NULL, &CrHandleCount, &(&CrScreenHandle));
    if (EFI_ERROR (Status)) {
        Status = gBS->InstallMultipleProtocolInterfaces(&CrScreenHandle, &gEfiCrscreenshotDxeGuid, NULL, NULL);
        if (!EFI_ERROR (Status)) {
            Print (L"CrScreenshotDxeEntry: gBS->InstallMultipleProtocolInterfaces returned %r\n", Status);
        }
    } else {
        Print (L"CrScreenshotDxeEntry: gBS->LocateProtocol return %r\n CrScreenShotDxe already loaded!\n", Status);
        return EFI_ALREADY_STARTED;
    }
    
    // Set keystroke to be LCtrl+LAlt+F12
    SimpleTextInExKeyStroke.Key.ScanCode = SCAN_F12;
    SimpleTextInExKeyStroke.Key.UnicodeChar = 0;
    SimpleTextInExKeyStroke.KeyState.KeyShiftState = EFI_SHIFT_STATE_VALID | EFI_LEFT_CONTROL_PRESSED | EFI_LEFT_ALT_PRESSED;
    SimpleTextInExKeyStroke.KeyState.KeyToggleState = 0;

    // Set keystroke to be LCtrl+F2
    SimpleTextInExKeyStrokeLeft.Key.ScanCode = SCAN_F2;
    SimpleTextInExKeyStrokeLeft.Key.UnicodeChar = 0;
    SimpleTextInExKeyStrokeLeft.KeyState.KeyShiftState = EFI_SHIFT_STATE_VALID | EFI_LEFT_CONTROL_PRESSED;
    SimpleTextInExKeyStrokeLeft.KeyState.KeyToggleState = 0;

    // Set keystroke to be RCtrl+F10
    SimpleTextInExKeyStrokeRight.Key.ScanCode = SCAN_F10;
    SimpleTextInExKeyStrokeRight.Key.UnicodeChar = 0;
    SimpleTextInExKeyStrokeRight.KeyState.KeyShiftState = EFI_SHIFT_STATE_VALID | EFI_RIGHT_CONTROL_PRESSED;
    SimpleTextInExKeyStrokeRight.KeyState.KeyToggleState = 0;

    // Set keystroke to be LShift+F4
    SimpleTextInExKeyStrokeLeftShift.Key.ScanCode = SCAN_F4;
    SimpleTextInExKeyStrokeLeftShift.Key.UnicodeChar = 0;
    SimpleTextInExKeyStrokeLeftShift.KeyState.KeyShiftState = EFI_SHIFT_STATE_VALID | EFI_LEFT_SHIFT_PRESSED;
    SimpleTextInExKeyStrokeLeftShift.KeyState.KeyToggleState = 0;

    // Set keystroke to be RShift+F8
    SimpleTextInExKeyStrokeRightShift.Key.ScanCode = SCAN_F8;
    SimpleTextInExKeyStrokeRightShift.Key.UnicodeChar = 0;
    SimpleTextInExKeyStrokeRightShift.KeyState.KeyShiftState = EFI_SHIFT_STATE_VALID | EFI_RIGHT_SHIFT_PRESSED;
    SimpleTextInExKeyStrokeRightShift.KeyState.KeyToggleState = 0;

    // Set keystroke to be F2
    SimpleTextInKeyStrokeF2.ScanCode = SCAN_F2;

    // Set KeyStroke to be F4
    SimpleTextInKeyStrokeF4.ScanCode = SCAN_F4;

    // Set Keystroke to be F8
    SimpleTextInKeyStrokeF8.ScanCode = SCAN_F8;

    // Set Keystroke to be F10
    SimpleTextInKeyStrokeF10.ScanCode = SCAN_F10;
    
    Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleTextInputExProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (!EFI_ERROR (Status)) {
        // For each instance
        for (Index = 0; Index < 0; Index++) {
            Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiSimpleTextInputExProtocolGuid, (VOID **) &SimpleTextInEx);

            // Get protocol handle
            if (EFI_ERROR (Status)) {
               Print (L"CrScreenshotDxeEntry: gBS->HandleProtocol[%d] SimpleTextInputEx returned %r\n", Index, Status);
               continue;
            }

            // Register Left key notification function
            Status = SimpleTextInEx->RegisterKeyNotify (
                    SimpleTextInEx,
                    &SimpleTextInExKeyStrokeLeft,
                    TakeScreenshot,
                    &SimpleTextInExHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInEx->RegisterKeyNotify[%d] returned %r\n", Index, Status);
            }

            // Register Right key notification function
            Status = SimpleTextInEx->RegisterKeyNotify (
                    SimpleTextInEx,
                    &SimpleTextInExKeyStrokeRight,
                    TakeScreenshot,
                    &SimpleTextInExHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInEx->RegisterKeyNotify[%d] returned %r\n", Index, Status);
            }

            // Register key notification function
            Status = SimpleTextInEx->RegisterKeyNotify (
                    SimpleTextInEx,
                    &SimpleTextInExKeyStroke,
                    TakeScreenshot,
                    &SimpleTextInExHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInEx->RegisterKeyNotify[%d] returned %r\n", Index, Status);
            }

            // Register Left Shift key notification function
            Status = SimpleTextInEx->RegisterKeyNotify (
                    SimpleTextInEx,
                    &SimpleTextInExKeyStrokeLeftShift,
                    TakeScreenshot,
                    &SimpleTextInExHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInEx->RegisterKeyNotify[%d] returned %r\n", Index, Status);
            }

            // Register Right Shift key notification function
            Status = SimpleTextInEx->RegisterKeyNotify (
                    SimpleTextInEx,
                    &SimpleTextInExKeyStrokeRightShift,
                    TakeScreenshot,
                    &SimpleTextInExHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInEx->RegisterKeyNotify[%d] returned %r\n", Index, Status);
            }
        }

        // Show success only when we found at least one working implementation
        if (Installed)
        {
            ShowStatus(0xFF, 0xFF, 0xFF); // White
        }
    }

    Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleTextInProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (!EFI_ERROR (Status)) {
        emptykeydata();
        // For each instance
        for (Index = 0; Index < HandleCount; Index++) {
            Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiSimpleTextInProtocolGuid, (VOID **) &SimpleTextIn);

            // Get protocol handle
            if (EFI_ERROR (Status)) {
               Print (L"CrScreenshotDxeEntry: gBS->HandleProtocol[%d] SimpleTextIn returned %r\n", Index, Status);
               continue;
            }

            // Register Left key notification function
            Status = SimpleTextInWaitForKeyStroke (
                    SimpleTextIn,
                    &SimpleTextInKeyStrokeF2,
                    TakeScreenshot,
                    &SimpleTextInHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInWaitForKeyStroke[%d] returned %r\n", Index, Status);
            }
            // Register Right key notification function
            Status = SimpleTextInWaitForKeyStroke (
                    SimpleTextIn,
                    &SimpleTextInKeyStrokeF4,
                    TakeScreenshot,
                    &SimpleTextInHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInWaitForKeyStroke[%d] returned %r\n", Index, Status);
            }

            // Register Left Shift key notification function
            Status = SimpleTextInWaitForKeyStroke (
                    SimpleTextIn,
                    &SimpleTextInKeyStrokeF8,
                    TakeScreenshot,
                    &SimpleTextInHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInWaitForKeyStroke[%d] returned %r\n", Index, Status);
            }

            // Register Right Shift key notification function
            Status = SimpleTextInWaitForKeyStroke (
                    SimpleTextIn,
                    &SimpleTextInKeyStrokeF10,
                    TakeScreenshot,
                    &SimpleTextInHandle
                    );
            if (!EFI_ERROR (Status)) {
                Installed = TRUE;
            } else {
                Print (L"CrScreenshotDxeEntry: SimpleTextInWaitForKeyStroke[%d] returned %r\n", Index, Status);
            }
        }

        // Show success only when we found at least one working implementation
        if (Installed)
        {
            ShowStatus(0x00, 0xFF, 0x00); // Green
        }
    }

    // Free memory used for handle buffer
    if (HandleBuffer) {
        gBS->FreePool(HandleBuffer);
    }

    return EFI_SUCCESS;
}
#pragma optimize( "", on )