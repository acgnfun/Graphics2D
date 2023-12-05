#include "Graphics2D.h"
#include <wincodec.h>
#include <wincodecsdk.h>

static ID2D1Factory* _G2D_Factory = nullptr;
static IDWriteFactory5* _G2D_WriteFactory = nullptr;
static IWICImagingFactory* _G2D_WICFactory = nullptr;

G2D_OBJECT::G2D_OBJECT()
{
	hWnd = nullptr;
	pFactory = nullptr;
	pTarget = nullptr;
	pWriteFactory = nullptr;
	bValid = false;
	CalcDpiScale();
}

G2D_OBJECT::~G2D_OBJECT()
{
	if (pTarget) pTarget->Release();
	pTarget = nullptr;
}

void G2D_OBJECT::Initialize(HWND hWnd)
{
	if (!_G2D_Factory) return;
	bValid = true;
	this->hWnd = hWnd;
	pFactory = _G2D_Factory;
	pWriteFactory = _G2D_WriteFactory;
	CalcDpiScale();
	RefreshResource();
}

void G2D_OBJECT::Uninitialize(G2D_CALLBACK RefreshResourceCallback)
{
	if (!bValid) return;
	if (RefreshResourceCallback) RefreshResourceCallback(nullptr);
	hWnd = nullptr;
	if (pTarget)
	{
		pTarget->Release();
		pTarget = nullptr;
	}
	pFactory = nullptr;
	pWriteFactory = nullptr;
	CalcDpiScale();
}

void G2D_OBJECT::Resize(int Width, int Height)
{
	if (!bValid) return;
	pTarget->Resize(
		D2D1::SizeU(
			Width,
			Height
		)
	);
}

bool G2D_OBJECT::RefreshResource()
{
	if (!bValid) return false;
	if (pTarget) pTarget->Release();
	RECT rc = { 0 };
	GetClientRect(hWnd, &rc);
	HRESULT hr = pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hWnd,
			D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top)
		),
		&pTarget
	);
	return !FAILED(hr);
}

bool G2D_OBJECT::BeginDraw()
{
	if (!bValid) return false;
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
	if (!pTarget) return false;
	pTarget->BeginDraw();
	return true;
}

bool G2D_OBJECT::DrawControl(D2D1_RECT_F Rect, LPCWSTR szText, IDWriteTextFormat* pTextFormat, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pBgBitmap)
{
	if (!bValid) return false;
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

bool G2D_OBJECT::DrawProgressBar(D2D1_RECT_F Rect, float Percentage, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BgColor, ID2D1Bitmap* pFrontBitmap, ID2D1Bitmap* pBgBitmap)
{
	if (!bValid) return false;
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

bool G2D_OBJECT::EndDraw(G2D_CALLBACK RefreshResourceCallback)
{
	if (!bValid) return false;
	if (!pTarget) return false;
	if (pTarget->EndDraw() == D2DERR_RECREATE_TARGET)
	{
		if (RefreshResourceCallback && !RefreshResourceCallback(nullptr))
			return false;
		return RefreshResource();
	}
	return true;
}

G2D_OBJECT::operator HWND()
{
	return hWnd;
}

G2D_OBJECT::operator ID2D1Factory* ()
{
	return pFactory;
}

G2D_OBJECT::operator ID2D1RenderTarget* ()
{
	return pTarget;
}

G2D_OBJECT::operator IDWriteFactory5* ()
{
	return pWriteFactory;
}

ID2D1Factory* G2D_OBJECT::Factory()
{
	return pFactory;
}

ID2D1RenderTarget* G2D_OBJECT::Target()
{
	return pTarget;
}

IDWriteFactory5* G2D_OBJECT::WFactory()
{
	return pWriteFactory;
}

void G2D_OBJECT::CalcDpiScale()
{
	if (bValid)
		fDpiScale = float(USER_DEFAULT_SCREEN_DPI) / GetDpiForWindow(hWnd);
	else
		fDpiScale = float(USER_DEFAULT_SCREEN_DPI) / GetDpiForSystem();
}

float G2D_OBJECT::DpiScale()
{
	return fDpiScale;
}

float G2D_OBJECT::PixelAdjust(int Origin)
{
	return Origin * fDpiScale;
}

bool G2D_OBJECT::Valid()
{
	return bValid;
}

void G2DInitialize()
{
	if (_G2D_Factory) return;
	if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_G2D_Factory) != S_OK)
		throw "g2da create graphics factory fail.";
	if (DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(_G2D_WriteFactory),
		reinterpret_cast<IUnknown**>(&_G2D_WriteFactory)
	) != S_OK)
		throw "g2da create write factory fail.";
}

void G2DUninitialize()
{
	if (_G2D_Factory)_G2D_Factory->Release();
	if (_G2D_WriteFactory)_G2D_WriteFactory->Release();
	_G2D_Factory = nullptr;
	_G2D_WriteFactory = nullptr;
}

void WICInitialize()
{
	if (_G2D_WICFactory) return;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&_G2D_WICFactory)
	);
}

void WICUnInitialize()
{
	if (_G2D_WICFactory) _G2D_WICFactory->Release();
	_G2D_WICFactory = nullptr;
}

HRESULT G2DCreateBitmap(ID2D1RenderTarget* pRenderTarget, LPCWSTR szPath, ID2D1Bitmap** ppBitmap)
{
	if (!_G2D_WICFactory) return E_FAIL;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	HRESULT hr = _G2D_WICFactory->CreateDecoderFromFilename(
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
		hr = _G2D_WICFactory->CreateFormatConverter(&pConverter);
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

HRESULT G2DCreateBitmap(ID2D1RenderTarget* pRenderTarget, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, ID2D1Bitmap** ppBitmap)
{
	if (!_G2D_WICFactory) return E_FAIL;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	IWICStream* pStream = nullptr;
	HRSRC hRscBitmap = nullptr;
	HGLOBAL hGlobalBitmap = nullptr;
	void* pImage = nullptr;
	DWORD nImageSize = 0;
	hRscBitmap = FindResourceW(hModule, szResourceName, szResourceType);
	HRESULT hr = hRscBitmap ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		hGlobalBitmap = LoadResource(hModule, hRscBitmap);
		hr = hGlobalBitmap ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImage = LockResource(hGlobalBitmap);
		hr = pImage ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		nImageSize = SizeofResource(hModule, hRscBitmap);
		hr = nImageSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = _G2D_WICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImage),
			nImageSize
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = _G2D_WICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = _G2D_WICFactory->CreateFormatConverter(&pConverter);
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
	if (pStream) pStream->Release();
	return hr;
}

HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, LPCWSTR szPath, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
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
	if (*BufElemNum <= len)
	{
		*BufElemNum = len + 1;
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pFFName->GetString(0, FontFamilyBuffer, *BufElemNum);
	if (pFSBuilder) pFSBuilder->Release();
	if (pFontSet) pFontSet->Release();
	if (pFontFile) pFontFile->Release();
	if (pFFamily) pFFamily->Release();
	if (pFFName) pFFName->Release();
	if (!SUCCEEDED(hr) && pFCollection) pFCollection->Release();
	if (SUCCEEDED(hr)) pFontCollection->pFontCollection = pFCollection;
	return hr;
}

HRESULT G2DCreateFontCollection(IDWriteFactory5* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, G2D_FONTCOLLECTION* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
{
	IDWriteFontSetBuilder1* pFSBuilder = nullptr;
	IDWriteFontSet* pFontSet = nullptr;
	IDWriteFontFile* pFontFile = nullptr;
	IDWriteFontCollection1* pFCollection = nullptr;
	IDWriteFontFamily* pFFamily = nullptr;
	IDWriteLocalizedStrings* pFFName = nullptr;
	HRSRC hRscFont = nullptr;
	HGLOBAL hGlobalFont = nullptr;
	void* pFontData = nullptr;
	DWORD nFDSize = 0;
	hRscFont = FindResourceW(hModule, szResourceName, szResourceType);
	HRESULT hr = hRscFont ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		hGlobalFont = LoadResource(hModule, hRscFont);
		hr = hGlobalFont ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pFontData = LockResource(hGlobalFont);
		hr = pFontData ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		nFDSize = SizeofResource(hModule, hRscFont);
		hr = nFDSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontSetBuilder(&pFSBuilder);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateInMemoryFontFileLoader(&pFontCollection->pMemoryLoader);
	if (SUCCEEDED(hr)) hr = pWriteFactory->RegisterFontFileLoader(pFontCollection->pMemoryLoader);
	if (SUCCEEDED(hr)) hr = pFontCollection->pMemoryLoader->CreateInMemoryFontFileReference(pWriteFactory, pFontData, nFDSize, nullptr, &pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->AddFontFile(pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->CreateFontSet(&pFontSet);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &pFCollection);
	if (SUCCEEDED(hr)) hr = pFCollection->GetFontFamily(0, &pFFamily);
	if (SUCCEEDED(hr)) hr = pFFamily->GetFamilyNames(&pFFName);
	UINT32 len = 0;
	if (SUCCEEDED(hr)) hr = pFFName->GetStringLength(0, &len);
	if (*BufElemNum <= len)
	{
		*BufElemNum = len + 1;
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pFFName->GetString(0, FontFamilyBuffer, *BufElemNum);
	if (pFSBuilder) pFSBuilder->Release();
	if (pFontSet) pFontSet->Release();
	if (pFontFile) pFontFile->Release();
	if (pFFamily) pFFamily->Release();
	if (pFFName) pFFName->Release();
	if (!SUCCEEDED(hr) && pFCollection) pFCollection->Release();
	if (SUCCEEDED(hr)) pFontCollection->pFontCollection = pFCollection;
	return hr;
}

G2D_FONTCOLLECTION::G2D_FONTCOLLECTION()
{
	pMemoryLoader = nullptr;
	pFontCollection = nullptr;
}

G2D_FONTCOLLECTION::~G2D_FONTCOLLECTION()
{
	pMemoryLoader = nullptr;
	pFontCollection = nullptr;
}

G2D_FONTCOLLECTION::operator IDWriteFontCollection1* ()
{
	return pFontCollection;
}

void G2D_FONTCOLLECTION::Release()
{
	if (pFontCollection) pFontCollection->Release();
	if (pMemoryLoader)
	{
		_G2D_WriteFactory->UnregisterFontFileLoader(pMemoryLoader);
		pMemoryLoader->Release();
	}
	pFontCollection = nullptr;
	pMemoryLoader = nullptr;
}
