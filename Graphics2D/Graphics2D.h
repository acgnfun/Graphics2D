#pragma once

#ifndef GRAPHICS2DV2_H
#define GRAPHICS2DV2_H

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

typedef bool(*G2D_CALLBACK)(void*);

class G2D_EXPORT G2D_OBJECT
{
public:
	G2D_OBJECT();
	~G2D_OBJECT();
	G2D_OBJECT(const G2D_OBJECT&) = delete;
	G2D_OBJECT& operator=(const G2D_OBJECT&) = delete;
	void Initialize(HWND hWnd);
	void Uninitialize(G2D_CALLBACK RefreshResourceCallback = nullptr);
	void Resize(int Width, int Height);
	bool RefreshResource();
	bool BeginDraw();
	bool DrawControl(D2D1_RECT_F Rect, LPCWSTR szText, IDWriteTextFormat* pTextFormat, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pBgBitmap = nullptr);
	bool DrawProgressBar(D2D1_RECT_F Rect, float Percentage, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pFrontBitmap = nullptr, ID2D1Bitmap* pBgBitmap = nullptr);
	bool EndDraw(G2D_CALLBACK RefreshResourceCallback = nullptr);
	operator HWND();
	operator ID2D1Factory* ();
	operator ID2D1RenderTarget* ();
	operator IDWriteFactory5* ();
	ID2D1Factory* Factory();
	ID2D1RenderTarget* Target();
	IDWriteFactory5* WFactory();
	void CalcDpiScale();
	float DpiScale();
	float PixelAdjust(int Origin);
	bool Valid();
private:
	HWND hWnd;
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pTarget;
	IDWriteFactory5* pWriteFactory;
	float fDpiScale;
	bool bValid;
};

class G2D_EXPORT G2D_FONTCOLLECTION
{
public:
	G2D_FONTCOLLECTION();
	~G2D_FONTCOLLECTION();
	G2D_FONTCOLLECTION(const G2D_FONTCOLLECTION&) = delete;
	G2D_FONTCOLLECTION& operator=(const G2D_FONTCOLLECTION&) = delete;
	operator IDWriteFontCollection1* ();
	void Release();
private:
	friend G2D_EXPORT HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, LPCWSTR szPath, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
	friend G2D_EXPORT HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);;
	IDWriteInMemoryFontFileLoader* pMemoryLoader;
	IDWriteFontCollection1* pFontCollection;
};

G2D_EXPORT void G2DInitialize();
G2D_EXPORT void G2DUninitialize();
G2D_EXPORT void WICInitialize();
G2D_EXPORT void WICUnInitialize();

G2D_EXPORT HRESULT G2DCreateBitmap(ID2D1RenderTarget* pRenderTarget, LPCWSTR szPath, ID2D1Bitmap** ppBitmap);
G2D_EXPORT HRESULT G2DCreateBitmap(ID2D1RenderTarget* pRenderTarget, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, ID2D1Bitmap** ppBitmap);
G2D_EXPORT HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, LPCWSTR szPath, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
G2D_EXPORT HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);

#endif // __cplusplus

#endif // !GRAPHICS2DV2_H
