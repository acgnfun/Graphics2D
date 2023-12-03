#pragma once

#ifndef GRAPHICS2DFRAME_H
#define GRAPHICS2DFRAME_H

#ifdef __cplusplus

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d2d1_3.h>
#include <dwrite_3.h>

#define G2D_EXPORT	__declspec(dllimport)
#ifdef G2D_BUILD
#undef G2D_EXPORT
#define G2D_EXPORT	__declspec(dllexport)
#endif // G2D_BUILD
#ifdef G2D_STATICLIB
#undef G2D_EXPORT
#define G2D_EXPORT
#endif // G2D_STATICLIB

#define CreateCentralWindow(lpClassName, lpWindowName, dwStyle, nWidth, nHeight,\
	hWndParent, hMenu, hInstance, lpParam)\
	CreateWindowExW(0L, lpClassName, lpWindowName, dwStyle, \
	(GetSystemMetrics(SM_CXSCREEN) - (nWidth)) / 2, \
	(GetSystemMetrics(SM_CYSCREEN) - (nHeight)) / 2, \
	nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
#define CreateCentralWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, nWidth, nHeight,\
	hWndParent, hMenu, hInstance, lpParam)\
	CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, \
	(GetSystemMetrics(SM_CXSCREEN) - (nWidth)) / 2, \
	(GetSystemMetrics(SM_CYSCREEN) - (nHeight)) / 2, \
	nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)

typedef bool(*G2DCallback)(void*);

class G2D_EXPORT G2DObject
{
public:
	G2DObject();
	~G2DObject();
	G2DObject(const G2DObject&) = delete;
	G2DObject& operator=(const G2DObject&) = delete;
	void Initialize(HWND hWnd);
	void Uninitialize(G2DCallback RefreshResourceCallback = nullptr);
	void Resize(int Width, int Height);
	bool RefreshResource();
	bool BeginDraw();
	bool DrawControl(D2D1_RECT_F Rect, LPCWSTR szText, IDWriteTextFormat* pTextFormat, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pBgBitmap = nullptr);
	bool DrawProgressBar(D2D1_RECT_F Rect, float Percentage, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pFrontBitmap = nullptr, ID2D1Bitmap* pBgBitmap = nullptr);
	bool EndDraw(G2DCallback RefreshResourceCallback = nullptr);
	HWND hWnd();
	ID2D1Factory* Factory();
	ID2D1RenderTarget* Target();
	IDWriteFactory* WriteFactory();
	float DpiScale();
	int PixelAdjust(int Origin);
private:
	HWND _hWnd;
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pTarget;
	IDWriteFactory5* pWriteFactory;
	float _DpiScale;
};

G2D_EXPORT void WICInitialize();
G2D_EXPORT void WICUnInitialize();

G2D_EXPORT HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, LPCWSTR szPath, ID2D1Bitmap** ppBitmap);
G2D_EXPORT HRESULT LoadBitmapFromResource(ID2D1RenderTarget* pRenderTarget, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, ID2D1Bitmap** ppBitmap);
G2D_EXPORT HRESULT CreateFontCollectionFromFile(IDWriteFactory5* pWriteFactory, LPCWSTR szPath, IDWriteFontCollection1** ppFontCollection, LPWSTR FontFamilyBuffer, UINT* BufSize);

#endif // __cplusplus

#endif // !GRAPHICS2DFRAME_H
