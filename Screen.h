#pragma once

#include <chrono>
#include "framework.h"
#include "atlimage.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#pragma comment(lib, "gdiplus.lib")

class Screen
{
public:
    Screen(HWND hwnd, int width, int height);
    void Update();
    void Cleanup();

private:
    void InitFramebuffer();
    void CaptureDesktop(int sectionNum, RGBQUAD* outBuffer);
    void UpdateFramebuffer(int sectionName);
    void Refresh(int sectionNum);

    int w, h;
    int fullW, fullH;
    HWND hWnd = NULL;
    Gdiplus::Bitmap* bmp = NULL;
    Gdiplus::Rect *rect = NULL;
    HDC memDC;
    HBITMAP memBitmap;
    SIZE windowSize;

    RGBQUAD* captureBuffer = NULL;
    RGBQUAD* lastCaptureBuffer = NULL;
    uint32_t* ageBuffer = NULL;
    
    int debugCount = 0;

    int currentSection = 0;
    int numSections = 1;
    int sectionStride = 0;
    int sectionHeight = 0;
    int sectionFullHeight = 0;
    int clearAmount = 0;
    float clearPercentage = 0.35f;
    int startDelay = 300;
    int darkestAlpha = 200; // Must be some value between 0 and 255.

    std::chrono::steady_clock::time_point lastUpdate;
};

