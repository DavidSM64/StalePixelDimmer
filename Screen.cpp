#include "Screen.h"

#define DEBUG_PRINT_UPDATE
//#define DEBUG_PRINT_CAPTUREDESKTOP
//#define DEBUG_PRINT_UPDATEFRAMEBUFFER
//#define DEBUG_PRINT_DRAWIMAGE

//#define DEBUG_OUTPUT_DESKTOP_CAPTURE

Screen::Screen(HWND hwnd, int width, int height)
{
    hWnd = hwnd;
    w = width;
    h = height;
	fullW = (int)(w * 1.75);
	fullH = (int)(h * 1.75);

	sectionStride = (w * h) / numSections; // Number of pixels in one section.
	clearAmount = (int)(sectionStride * clearPercentage);

	sectionHeight = h / numSections;
	sectionFullHeight = fullH / numSections;

	captureBuffer = new RGBQUAD[sectionStride];
	lastCaptureBuffer = new RGBQUAD[w * h];
	ageBuffer = new uint32_t[w * h];
	memset(ageBuffer, 0, w * h * sizeof(uint32_t)); // zero out ageBuffer.

	InitFramebuffer();
	Refresh(0);
	
	lastUpdate = std::chrono::steady_clock::now();
}

void Screen::Cleanup()
{
	DeleteDC(memDC);
	DeleteObject(memBitmap);
	delete[] captureBuffer;
}

void Screen::Update()
{
	// Keep window on top. Not sure if this is necessary?
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	if (std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - lastUpdate).count() > 120) {
#ifdef DEBUG_PRINT_UPDATE
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

			UpdateFramebuffer(currentSection);
			Refresh(currentSection);
			currentSection = (currentSection + 1) % numSections;
			if (currentSection == 0) {
				debugCount++;
			}

#ifdef DEBUG_PRINT_UPDATE
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		uint64_t durMs = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
		printMsg("Update took %llu ms\n", durMs);
#endif
		lastUpdate = std::chrono::steady_clock::now();
	}
}

void Screen::InitFramebuffer()
{

	// PixelFormat32bppPARGB speeds up drawing time.
	bmp = new Gdiplus::Bitmap(w, h, PixelFormat32bppPARGB);

	Gdiplus::BitmapData bitmapData;
	rect = new Gdiplus::Rect(0, 0, bmp->GetWidth(), bmp->GetHeight());

	if (bmp->LockBits(rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, bmp->GetPixelFormat(), &bitmapData) != Gdiplus::Ok)
	{
		throw 1; // TODO: better errors!
	}

	int* buffer = (int*)bitmapData.Scan0;

	for (int y = 0; y < h; y++) 
	{
		for (int x = 0; x < w; x++) 
		{
			buffer[((y * w) + x)] = 0;
		}
	}

	bmp->UnlockBits(&bitmapData);

	RECT wndRect;
	GetWindowRect(hWnd, &wndRect);
	windowSize = {
		wndRect.right - wndRect.left,
		wndRect.bottom - wndRect.top
	};
	HDC hdc = GetDC(hWnd);

	memDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, windowSize.cx, windowSize.cy);

	// prefill lastCaptureBuffer
	for (int i = 0; i < numSections; i++) {
		CaptureDesktop(i, &lastCaptureBuffer[i * sectionStride]);
	}
}

// Gets the pixels of the section, and copies them to the capture buffer.
// This code is based off: https://stackoverflow.com/q/2659932
void Screen::CaptureDesktop(int sectionNum, RGBQUAD* outBuffer)
{

	HWND hDesktopWnd = GetDesktopWindow();
	HDC hDesktopDC = GetDC(hDesktopWnd);
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, w, sectionHeight);
	SelectObject(hCaptureDC, hCaptureBitmap);

#ifdef DEBUG_PRINT_CAPTUREDESKTOP
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	int xDest = 0;
	int yDest = 0;
	int wDest = w;
	int hDest = sectionHeight;
	int xSrc = 0;
	int ySrc = sectionNum * sectionFullHeight;
	int wSrc = fullW;
	int hSrc = sectionFullHeight;

	// This function is slow! Is there a faster way to do this?
	StretchBlt(hCaptureDC, xDest, yDest, wDest, hDest, hDesktopDC, xSrc, ySrc, wSrc, hSrc, SRCCOPY | CAPTUREBLT | HALFTONE);

#ifdef DEBUG_PRINT_CAPTUREDESKTOP
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	uint64_t durMs = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
	printMsg("CaptureDesktop took %llu ms\n", durMs);
#endif

	Gdiplus::Bitmap *desktopBmp = Gdiplus::Bitmap::FromHBITMAP(hCaptureBitmap, NULL);

	Gdiplus::BitmapData bitmapData;

	Gdiplus::Rect rect2(0, 0, desktopBmp->GetWidth(), desktopBmp->GetHeight());


#ifdef DEBUG_OUTPUT_DESKTOP_CAPTURE
	CLSID pngEncoder = { 0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} };
	WCHAR filenamew[64];
	swprintf_s(filenamew, L"test%d.png", sectionNum);
	Gdiplus::Bitmap(hCaptureBitmap, NULL).Save(filenamew, &pngEncoder);
#endif
	int status = desktopBmp->LockBits(&rect2, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, desktopBmp->GetPixelFormat(), &bitmapData);
	if (status != Gdiplus::Ok)
	{
		throw status; // TODO: better errors!
	}

	// Desktop bytes will be RGB24! No alpha!

	int stride = abs(bitmapData.Stride);
	int width = bitmapData.Width;
	int height = bitmapData.Height;
	BYTE* bmpAddr = (BYTE*)bitmapData.Scan0;

	for (int y = 0; y < height; y++) {
		// Stride can be positive or negative, so have to memcpy by line!
		memcpy(&outBuffer[y * width], bmpAddr, width * 4);
		bmpAddr += bitmapData.Stride; 
	}

	desktopBmp->UnlockBits(&bitmapData);

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
	delete desktopBmp;
}

void Screen::UpdateFramebuffer(int sectionNum)
{

	Gdiplus::BitmapData bitmapData;

	if (bmp->LockBits(rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, bmp->GetPixelFormat(), &bitmapData) != Gdiplus::Ok)
	{
		throw 1; // TODO: better errors!
	}

	int* buffer = (int*)bitmapData.Scan0;

	int startIndex = sectionNum * sectionStride;
	int endIndex = startIndex + sectionStride;

	//printMsg("sectionNum = %d, start = %d, end = %d\n", sectionNum, startIndex, endIndex);

	CaptureDesktop(sectionNum, captureBuffer); // slow!

#ifdef DEBUG_PRINT_UPDATEFRAMEBUFFER
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
	int diffCount = 0;
	for (int i = startIndex; i < endIndex; i++) {
		// Get the difference between this pixel and the previous.
		bool diff = (((int*)lastCaptureBuffer)[i] - ((int*)captureBuffer)[i - startIndex]) != 0;
		diffCount += diff;
		ageBuffer[i] = diff ? 0 : (ageBuffer[i] + 2); // I'm hoping this doesn't turn into an if/else, but a select operation.
		buffer[i] = (min(max(((int)ageBuffer[i]), 0), 255) & 0xFF) << 24;
	}
	//printMsg("DiffCount = %d\n", diffCount);

	if (diffCount > clearAmount) {
		memset(&buffer[startIndex], 0, sectionStride * sizeof(RGBQUAD));
		memset(&ageBuffer[startIndex], 0, sectionStride * sizeof(uint32_t));
	}

#ifdef DEBUG_PRINT_UPDATEFRAMEBUFFER
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	uint64_t durMs = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
	printMsg("UpdateFramebuffer took %llu ms\n", durMs);
#endif

	bmp->UnlockBits(&bitmapData);
}

void Screen::Refresh(int sectionNum)
{
	if (sectionNum >= numSections) {
		return;
	}

	SelectObject(memDC, memBitmap);
	Gdiplus::Graphics graphics(memDC);

#ifdef DEBUG_PRINT_DRAWIMAGE
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
	// Overwrite mask pixels instead of drawing over them!
	graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);

	int bmph = (windowSize.cy / numSections);
	int bmpy = sectionNum * bmph;

	// This function is slow! Is there a faster way to do this?
	graphics.DrawImage(bmp, 0, bmpy, 0, bmpy, windowSize.cx, bmph + 1, Gdiplus::UnitPixel);

#ifdef DEBUG_PRINT_DRAWIMAGE
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	uint64_t durMs = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
	printMsg("DrawImage took %llu ms\n", durMs);
#endif

	HDC screenDC = GetDC(NULL);
	POINT pointSrc = { 0,0 };

	BLENDFUNCTION blendFunction;
	blendFunction.AlphaFormat = AC_SRC_ALPHA;
	blendFunction.BlendFlags = 0;
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.SourceConstantAlpha = darkestAlpha;
	UpdateLayeredWindow(hWnd, screenDC, &pointSrc, &windowSize, memDC, &pointSrc, 0, &blendFunction, 2);

	CaptureDesktop(sectionNum, &lastCaptureBuffer[sectionNum * sectionStride]);

	//throw 1;
}
