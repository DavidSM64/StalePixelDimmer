#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

#include "framework.h"
char printBuffer[256];

#include "atlimage.h"
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include "Screen.h"
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Ole32.lib")

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.Lib")
#pragma comment(lib, "gdi32.Lib")

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
LPCWSTR szClassName = L"SPD";

Screen* screen;

int WINAPI WinMain(HINSTANCE hThisInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nCmdShow)
{
	/**/
	/*  GDI+ startup token */
	ULONG_PTR gdiplusStartupToken;
	Gdiplus::GdiplusStartupInput gdiInput;
	Gdiplus::GdiplusStartup(&gdiplusStartupToken, &gdiInput, NULL);
	MSG messages;            /* Here messages to the application are saved */
	{
		/**/
		HWND hwnd;               /* This is the handle for our window */
		WNDCLASSEX wincl;        /* Data structure for the windowclass */

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		/* The Window structure */
		wincl.hInstance = hThisInstance;
		wincl.lpszClassName = (LPCWSTR)szClassName;//+-69+
		wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
		wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
		wincl.cbSize = sizeof(WNDCLASSEX);

		/* Use default icon and mouse-pointer */
		wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wincl.lpszMenuName = NULL;                 /* No menu */
		wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
		wincl.cbWndExtra = 0;                      /* structure or the window instance */
		/* Use Windows's default colour as the background of the window */
		wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

		/* Register the window class, and if it fails quit the program */
		if (!RegisterClassEx(&wincl))
			return 0;

		/* The class is registered, let's create the program*/
		hwnd = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_TRANSPARENT,                   /* Extended possibilites for variation */
			(LPCWSTR)szClassName,         /* Classname */
			(LPCWSTR)szClassName,       /* Title Text */
			WS_OVERLAPPEDWINDOW, /* default window */
			CW_USEDEFAULT,       /* Windows decides the position */
			CW_USEDEFAULT,       /* where the window ends up on the screen */
			screenWidth,                 /* The programs width */
			screenHeight,                 /* and height in pixels */
			HWND_DESKTOP,        /* The window is a child-window to desktop */
			NULL,                /* No menu */
			hThisInstance,       /* Program Instance handler */
			NULL                 /* No Window Creation data */
		);

		/* Make the window visible on the screen */
		ShowWindow(hwnd, nCmdShow);


		screen = new Screen(hwnd, screenWidth, screenHeight);

		/*
		bmp = new Gdiplus::Bitmap(screenWidth, screenHeight);
		init_maskbuffer(bmp, screenWidth, screenHeight);

		refresh_framebuffer(hwnd);
		*/

		SetTimer(hwnd, 0, 1, NULL);

		/* Run the message loop. It will run until GetMessage() returns 0 */
		while (GetMessage(&messages, NULL, 0, 0))
		{
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&messages);
			/* Send message to WindowProcedure */
			DispatchMessage(&messages);
		}
	}
	Gdiplus::GdiplusShutdown(gdiplusStartupToken);
	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)                  /* handle the messages */
	{
	case WM_TIMER:
		{
			screen->Update();
		}
		break;
	case WM_DESTROY:
		screen->Cleanup();
		PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
		break;
	case WM_LBUTTONDOWN:
		printf("LButtonDown");
		//::PostMessage(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,lParam);
		break;
	default:                      /* for messages that we don't deal with */
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}