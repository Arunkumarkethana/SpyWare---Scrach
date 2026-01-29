#include "core/collection/screenshot.hpp"
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <memory>

#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

// Stream implementation for GDI+ Save
class MemStream : public IStream {
    // Minimal standard implementation for IStream needed by GDI+
    // Since implementing full COM interface is verbose, we'll cheat or use SHCreateMemStream if available (shlwapi)
    // Or we use a simple global helper. 
    // Actually, CreateStreamOnHGlobal is standard and easy.
};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    GetImageEncodersSize(&num, &size);
    if(size == 0) return -1;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j) {
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}

std::vector<uint8_t> Screenshot::Capture() {
    std::vector<uint8_t> buffer;
    
    // GDI+ should be initialized externally before calling this
    // Get Desktop DC
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hbmScreen);
    
    // BitBlt
    if(!BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY)) {
         // CLEANUP BEFORE EXIT (A++ Resource Safety)
         DeleteObject(hbmScreen);
         DeleteDC(hdcMem);
         ReleaseDC(NULL, hdcScreen);
         return buffer;
    }
    
    // Save to stream
    Bitmap bitmap(hbmScreen, NULL);
    IStream* pStream = NULL;
    if(CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK) {
        CLSID clsid;
        if(GetEncoderClsid(L"image/jpeg", &clsid) != -1) {
            bitmap.Save(pStream, &clsid, NULL);
            
            // Get content
            LARGE_INTEGER liZero = {};
            ULARGE_INTEGER liSize;
            pStream->Seek(liZero, STREAM_SEEK_END, &liSize);
            pStream->Seek(liZero, STREAM_SEEK_SET, NULL);
            
            size_t size = (size_t)liSize.QuadPart;
            buffer.resize(size);
            
            ULONG bytesRead;
            pStream->Read(buffer.data(), size, &bytesRead);
        }
        pStream->Release();
    }
    
    DeleteObject(hbmScreen);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return buffer;
}
