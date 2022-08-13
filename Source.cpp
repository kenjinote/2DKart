#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include "resource.h"

TCHAR szClassName[] = TEXT("Window");
HWND m_hwnd;
ID2D1Factory* m_pD2DFactory;
IWICImagingFactory* m_pWICFactory;
IDWriteFactory* m_pDWriteFactory;
ID2D1HwndRenderTarget* m_pRenderTarget;
IDWriteTextFormat* m_pTitleTextFormat;
ID2D1SolidColorBrush* m_pBlackBrush;
ID2D1SolidColorBrush* m_pRedBrush;
ID2D1SolidColorBrush* m_pBlueBrush;
ID2D1SolidColorBrush* m_pGrayBrush;
ID2D1SolidColorBrush* m_pGrayRedBrush;
ID2D1SolidColorBrush* m_pGrayBlueBrush;
ID2D1Bitmap* m_pBitmap;
DWORD dwSpeed;

HRESULT LoadResourceBitmap(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = NULL;
	DWORD imageFileSize = 0;
	imageResHandle = FindResourceW(GetModuleHandle(0), resourceName, resourceType);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		imageResDataHandle = LoadResource(GetModuleHandle(0), imageResHandle);
		hr = imageResDataHandle ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		pImageFile = LockResource(imageResDataHandle);
		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		imageFileSize = SizeofResource(GetModuleHandle(0), imageResHandle);
		hr = imageFileSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateDecoderFromStream(pStream, 0, WICDecodeMetadataCacheOnLoad, &pDecoder);
	}
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeMedianCut);
	}
	if (SUCCEEDED(hr))
	{
		hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, 0, ppBitmap);
	}
	pDecoder->Release();
	pDecoder = NULL;
	pSource->Release();
	pSource = NULL;
	pStream->Release();
	pStream = NULL;
	pConverter->Release();
	pConverter = NULL;
	return hr;
}

HRESULT CreateDeviceIndependentResources()
{
	static const FLOAT msc_fontSize = 64;
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWriteFactory));
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pDWriteFactory->CreateTextFormat(L"メイリオ", 0, DWRITE_FONT_WEIGHT_ULTRA_BLACK/*DWRITE_FONT_WEIGHT_NORMAL*/, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, msc_fontSize, L"", &m_pTitleTextFormat);
	}
	if (SUCCEEDED(hr))
	{
		m_pTitleTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_pTitleTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	}
	if (SUCCEEDED(hr))
	{
		CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&m_pWICFactory));
	}
	return hr;
}

HRESULT CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_pRenderTarget);
		if (SUCCEEDED(hr))
		{
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(0.0f, 0.0f, 0.0f, 0.70f)), &m_pBlackBrush);
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(1.0f, 0.0f, 0.0f, 0.70f)), &m_pRedBrush);
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(0.0f, 0.0f, 1.0f, 0.70f)), &m_pBlueBrush);
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(0.0f, 0.0f, 0.0f, 0.15f)), &m_pGrayBrush);
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(1.0f, 0.0f, 0.0f, 0.15f)), &m_pGrayRedBrush);
			m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::ColorF(0.0f, 0.0f, 1.0f, 0.15f)), &m_pGrayBlueBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = LoadResourceBitmap(m_pRenderTarget, m_pWICFactory, MAKEINTRESOURCE(IDB_PNG1), L"PNG", 0, 0, &m_pBitmap);
		}
	}
	return hr;
}

void DiscardDeviceResources()
{
	m_pRenderTarget->Release();
	m_pRenderTarget = NULL;
	m_pWICFactory->Release();
	m_pWICFactory = NULL;
	m_pBlackBrush->Release();
	m_pBlackBrush = NULL;
	m_pRedBrush->Release();
	m_pRedBrush = NULL;
	m_pBlueBrush->Release();
	m_pBlueBrush = NULL;
	m_pGrayBrush->Release();
	m_pGrayBrush = NULL;
	m_pGrayRedBrush->Release();
	m_pGrayRedBrush = NULL;
	m_pGrayBlueBrush->Release();
	m_pGrayBlueBrush = NULL;
	m_pBitmap->Release();
	m_pBitmap = NULL;
}

// 描画関数
HRESULT OnRender(float rotate)
{
	HRESULT hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{
		static TCHAR szText[128];
		wsprintf(szText, TEXT("speed: %d km"), dwSpeed);
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		D2D1_SIZE_F size = m_pBitmap->GetSize();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(rotate, D2D1::Point2F(renderTargetSize.width / 2.0f, renderTargetSize.height / 2.0f)));
		m_pRenderTarget->DrawBitmap(m_pBitmap, D2D1::RectF(renderTargetSize.width / 2.0f + size.width, renderTargetSize.height / 2.0f + size.width, size.width, size.height), 1.0f);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->DrawText(szText, lstrlen(szText), m_pTitleTextFormat, D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), m_pBlackBrush);
		hr = m_pRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}
	return hr;
}

void OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;
		m_pRenderTarget->Resize(size);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static float rotate = 0.0f;
	switch (msg)
	{
	case WM_CREATE:
		break;
	case WM_SIZE:
		OnResize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpMMI = (MINMAXINFO*)lParam;
			lpMMI->ptMaxTrackSize.x = 1024;
			lpMMI->ptMaxTrackSize.y = 768;
			return 0;
		}
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_LEFT:
				rotate += 2.5f;
				InvalidateRect(hWnd, 0, 0);
				break;
			case VK_RIGHT:
				rotate -= 2.5f;
				InvalidateRect(hWnd, 0, 0);
				break;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
		}
		break;
	case WM_DISPLAYCHANGE:
		InvalidateRect(hWnd, 0, 0);
		break;
	case WM_PAINT:
		OnRender(rotate);
		ValidateRect(hWnd, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	MSG msg = {};
	(void)CoInitialize(0);
	CreateDeviceIndependentResources();
	WNDCLASS wndclass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, 0, LoadCursor(0, IDC_ARROW), 0, 0, szClassName };
	RegisterClass(&wndclass);
	m_hwnd = CreateWindow(szClassName, TEXT("2D Kart"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0, 0, hInstance, 0);
	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	m_pD2DFactory->Release();
	m_pD2DFactory = NULL;
	m_pDWriteFactory->Release();
	m_pDWriteFactory = NULL;
	m_pTitleTextFormat->Release();
	m_pTitleTextFormat = NULL;
	DiscardDeviceResources();
	CoUninitialize();
	return (int)msg.wParam;
}
