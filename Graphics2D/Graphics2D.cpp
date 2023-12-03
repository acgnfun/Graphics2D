#include "Graphics2D.h"
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <wincodecsdk.h>

static unsigned long long _gd2a_counter = 0;
ID2D1Factory* _gd2a_graphics_pFactory = nullptr;
IDWriteFactory5* _gd2a_write_pFactory = nullptr;
IWICImagingFactory* _wic_pFactory = nullptr;

static void _gd2a_Init()
{
	if (_gd2a_counter == 0)
	{
		CoInitialize(nullptr);
		if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_gd2a_graphics_pFactory) != S_OK)
			throw "g2da create graphics factory fail.";
		if (DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(_gd2a_write_pFactory),
			reinterpret_cast<IUnknown**>(&_gd2a_write_pFactory)
		) != S_OK)
			throw "g2da create write factory fail.";
	}
	_gd2a_counter++;
}

static void _gd2a_Uninit()
{
	if (_gd2a_counter == 0) return;
	_gd2a_counter--;
	if (_gd2a_counter == 0)
	{
		if (_gd2a_graphics_pFactory)_gd2a_graphics_pFactory->Release();
		if (_gd2a_write_pFactory)_gd2a_write_pFactory->Release();
		_gd2a_graphics_pFactory = nullptr;
		_gd2a_write_pFactory = nullptr;
		//CoUninitialize();
	}
}

G2DObject::G2DObject()
{
	_hWnd = nullptr;
	pFactory = nullptr;
	pTarget = nullptr;
	pWriteFactory = nullptr;
	_DpiScale = float(USER_DEFAULT_SCREEN_DPI) / GetDpiForSystem();
}

G2DObject::~G2DObject()
{
	if (pTarget) pTarget->Release();
}

void G2DObject::Initialize(HWND hWnd)
{
	_gd2a_Init();
	_hWnd = hWnd;
	pFactory = _gd2a_graphics_pFactory;
	_DpiScale = float(USER_DEFAULT_SCREEN_DPI) / GetDpiForWindow(hWnd);
	pWriteFactory = _gd2a_write_pFactory;
	RefreshResource();
}

void G2DObject::Uninitialize(G2DCallback RefreshResourceCallback)
{
	if (RefreshResourceCallback) RefreshResourceCallback(nullptr);
	_hWnd = nullptr;
	if (pTarget)
	{
		pTarget->Release();
		pTarget = nullptr;
	}
	pFactory = nullptr;
	_DpiScale = 0.0f;
	_gd2a_Uninit();
}

void G2DObject::Resize(int Width, int Height)
{
	pTarget->Resize(
		D2D1::SizeU(
			Width,
			Height
		)
	);
}

bool G2DObject::RefreshResource()
{
	if (pTarget) pTarget->Release();
	RECT rc = { 0 };
	GetClientRect(_hWnd, &rc);
	HRESULT hr = pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			_hWnd,
			D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top)
		),
		&pTarget
	);
	return !FAILED(hr);
}

bool G2DObject::BeginDraw()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(_hWnd, &ps);
	// TODO: 在此处添加使用 hdc 的任何绘图代码...
	EndPaint(_hWnd, &ps);
	if (!pTarget) return false;
	pTarget->BeginDraw();
	return true;
}

bool G2DObject::DrawControl(D2D1_RECT_F Rect, LPCWSTR szText, IDWriteTextFormat* pTextFormat, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pBgBitmap)
{
	if (szText && !pTextFormat) return false;
	ID2D1SolidColorBrush* pBrush = nullptr;
	if (pBgBitmap)
	{
		pTarget->DrawBitmap(pBgBitmap, Rect);
	}
	else
	{
		pTarget->CreateSolidColorBrush(BgColor, &pBrush);
		pTarget->FillRectangle(Rect, pBrush);
		pBrush->Release();
	}
	if (szText)
	{
		pTarget->CreateSolidColorBrush(FrontColor, &pBrush);
		pTarget->DrawTextW(szText, wcslen(szText), pTextFormat, Rect, pBrush);
		pBrush->Release();
	}
	return true;
}

bool G2DObject::DrawProgressBar(D2D1_RECT_F Rect, float Percentage, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pFrontBitmap, ID2D1Bitmap* pBgBitmap)
{
	ID2D1SolidColorBrush* pBrush = nullptr;
	if (pBgBitmap)
		pTarget->DrawBitmap(pBgBitmap, Rect);
	else
	{
		pTarget->CreateSolidColorBrush(BgColor, &pBrush);
		pTarget->FillRectangle(Rect, pBrush);
		pBrush->Release();
		pBrush = nullptr;
	}
	Rect.right = Rect.left + ((Rect.right - Rect.left) * Percentage);
	if (pFrontBitmap)
		pTarget->DrawBitmap(pFrontBitmap, Rect);
	else
	{
		pTarget->CreateSolidColorBrush(FrontColor, &pBrush);
		pTarget->FillRectangle(Rect, pBrush);
		pBrush->Release();
		pBrush = nullptr;
	}
	return true;
}

bool G2DObject::EndDraw(G2DCallback RefreshResourceCallback)
{
	if (!pTarget) return false;
	if (pTarget->EndDraw() == D2DERR_RECREATE_TARGET)
	{
		if (RefreshResourceCallback && !RefreshResourceCallback(nullptr))
			return false;
		return RefreshResource();
	}
	return true;
}

HWND G2DObject::hWnd()
{
	return _hWnd;
}

ID2D1Factory* G2DObject::Factory()
{
	return pFactory;
}

ID2D1RenderTarget* G2DObject::Target()
{
	return pTarget;
}

IDWriteFactory* G2DObject::WriteFactory()
{
	return pWriteFactory;
}

float G2DObject::DpiScale()
{
	return _DpiScale;
}

int G2DObject::PixelAdjust(int Origin)
{
	return Origin * _DpiScale;
}

void WICInitialize()
{
	if (_wic_pFactory) return;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&_wic_pFactory)
	);
}

void WICUnInitialize()
{
	if (_wic_pFactory) _wic_pFactory->Release();
	_wic_pFactory = nullptr;
}

HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, LPCWSTR szPath, ID2D1Bitmap** ppBitmap)
{
	if (!_wic_pFactory) return E_FAIL;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	HRESULT hr = _wic_pFactory->CreateDecoderFromFilename(
		szPath,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = _wic_pFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitm
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}
	if (pDecoder) pDecoder->Release();
	if (pSource) pSource->Release();
	if (pConverter) pConverter->Release();
	return hr;
}

HRESULT CreateFontCollectionFromFile(IDWriteFactory5* pWriteFactory, LPCWSTR szPath, IDWriteFontCollection1** ppFontCollection, LPWSTR FontFamilyBuffer, UINT* BufSize)
{
	IDWriteFontSetBuilder1* pFSBuilder = nullptr;
	IDWriteFontSet* pFontSet = nullptr;
	IDWriteFontFile* pFontFile = nullptr;
	IDWriteFontCollection1* pFCollection = nullptr;
	IDWriteFontFamily* pFFamily = nullptr;
	IDWriteLocalizedStrings* pFFName = nullptr;
	HRESULT hr = pWriteFactory->CreateFontSetBuilder(&pFSBuilder);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontFileReference(szPath, /* lastWriteTime*/ nullptr, &pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->AddFontFile(pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->CreateFontSet(&pFontSet);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &pFCollection);
	if (SUCCEEDED(hr)) hr = pFCollection->GetFontFamily(0, &pFFamily);
	if (SUCCEEDED(hr)) hr = pFFamily->GetFamilyNames(&pFFName);
	UINT32 len = 0;
	if (SUCCEEDED(hr)) hr = pFFName->GetStringLength(0, &len);
	if (*BufSize <= len)
	{
		*BufSize = len + 1;
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pFFName->GetString(0, FontFamilyBuffer, *BufSize);
	if (pFSBuilder) pFSBuilder->Release();
	if (pFontSet) pFontSet->Release();
	if (pFontFile) pFontFile->Release();
	if (pFFamily) pFFamily->Release();
	if (pFFName) pFFName->Release();
	if (!SUCCEEDED(hr) && pFCollection) pFCollection->Release();
	if (SUCCEEDED(hr)) *ppFontCollection = pFCollection;
	return hr;
}
