// DXGIDuplicator.cpp

#include "opDXGI.h"
#include "./core/globalVar.h"
#include "./core/helpfunc.h"
#include "./core/opEnv.h"
#include "./include/Image.hpp"
#include <iostream>
#include <string>

opDXGI::opDXGI():device_(nullptr), deviceContext_(nullptr),
duplication_(nullptr), m_first(true), m_frameInfo(),dx_(0),dy_(0),m_desc()
{
}

opDXGI::~opDXGI()
{
	UnBindEx();
}


long opDXGI::BindEx(HWND _hwnd, long render_type) {
	if (!InitD3D11Device())
	{
		setlog("Init d3d11 device failed");
		return 0;
	}

	if (!InitDuplication())
	{
		setlog("Init duplication failed");
		return 0;
	}
	RECT rc, rc2;
	::GetWindowRect(_hwnd, &rc);
	::GetClientRect(_hwnd, &rc2);

	_width = rc2.right - rc2.left;
	_height = rc2.bottom - rc2.top;
	POINT pt = { 0 };
	::ClientToScreen(_hwnd, &pt);
	dx_ = pt.x - rc.left;
	dy_ = pt.y - rc.top;
	return 1;
}

long opDXGI::UnBindEx() {
	if (duplication_)
	{
		duplication_->Release();
		duplication_ = nullptr;
	}
	if (device_)
	{
		device_->Release();
		device_ = nullptr;
	}
	if (deviceContext_)
	{
		deviceContext_->Release();
		deviceContext_ = nullptr;
	}
	return 0;
}

bool opDXGI::requestCapture(int x1, int y1, int w, int h, Image& img) {
	img.create(w, h);
	ID3D11Texture2D* texture2D = nullptr;
	uint8_t* pDest = _shmem->data<byte>() + sizeof(FrameInfo);
	if (!GetDesktopFrame(&texture2D))
	{
		setlog("Acquire frame failed");
		return false;
	}
	if (texture2D == nullptr)
	{
		setlog("Acquire frame timeout");
		// we do not update and use previous frame
	}
	else {
		//copy screen data to memory
		texture2D->GetDesc(&m_desc);
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		deviceContext_->Map(texture2D, 0, D3D11_MAP_READ, 0, &mappedResource);
		size_t imageSize = m_desc.Width * m_desc.Height * 4;

		uint8_t* pData = (uint8_t*)mappedResource.pData;
		_pmutex->lock();
		fmtFrameInfo(_shmem->data<byte>(), _hwnd, w, h);

		// size_t imageSize1 = m_desc.Width * m_desc.Height * 4;
		// uint8_t* rgba = (uint8_t*)malloc(imageSize1);
		// if (rgba == nullptr)
		// {
		// 	return 0;
		// }
		// memset(rgba, 0, imageSize1);
		// for (size_t i = 0; i < m_desc.Height; i++)
		// {
		// 	memcpy(rgba + i * m_desc.Width * 4, pData + i * mappedResource.RowPitch, m_desc.Width * 4);
		// }
		// SaveBmp("testbmp.bmp", rgba, m_desc.Width, m_desc.Height);
		// free(rgba);

		for (size_t i = 0; i < m_desc.Height; i++)
		{
			memcpy(pDest + i * m_desc.Width * 4, pData + i * mappedResource.RowPitch, m_desc.Width * 4);
		}
		_pmutex->unlock();
		texture2D->Release();
		texture2D = nullptr;
	}
	

	RECT rc;
	::GetWindowRect(_hwnd, &rc);
	int src_x = x1 + rc.left + dx_;
	int src_y = y1 + rc.top + dy_;
	if (src_x + w > m_desc.Width || src_y + h > m_desc.Height) {
		setlog("error w and h src_x=%d,w=%d,desc.Width=%d,src_y=%d,h=%d,desc.Height=%d", src_x, w, m_desc.Width, src_y, h, m_desc.Height);
		return false;
	}
	//copy memory to image
	_pmutex->lock();
	uchar* pshare = _shmem->data<byte>();
	
	//将数据拷贝到目标
	for (int i = 0; i < h; i++) {
		//memcpy(img.ptr<uchar>(i), pDest + (desc.Height - 1 - i - src_y) * 4 * desc.Width + src_x * 4,
			//4 * w);
		memcpy(img.ptr<uchar>(i), pDest + (src_y+i) * 4 * m_desc.Width + src_x * 4,
			4 * w);
	}
	_pmutex->unlock();
	return true;
}

void opDXGI::SaveBmp(std::string filename, const uint8_t* data, int width, int height)
{
    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == NULL)
    {
        return;
    }
    // 已写入字节数
    DWORD bytesWritten = 0;
    // 位图大小，颜色默认为32位即rgba
    int bmpSize = width * height * 4;

    // 文件头
    BITMAPFILEHEADER bmpHeader;
    // 文件总大小 = 文件头 + 位图信息头 + 位图数据
    bmpHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpSize;
    // 固定
    bmpHeader.bfType = 0x4D42;
    // 数据偏移，即位图数据所在位置
    bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    // 保留为0
    bmpHeader.bfReserved1 = 0;
    // 保留为0
    bmpHeader.bfReserved2 = 0;
    // 写文件头
    WriteFile(hFile, (LPSTR)&bmpHeader, sizeof(bmpHeader), &bytesWritten, NULL);

    // 位图信息头
    BITMAPINFOHEADER bmiHeader;
    // 位图信息头大小
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    // 位图像素宽度
    bmiHeader.biWidth = width;
    // 位图像素高度，负高度即上下翻转
    bmiHeader.biHeight = -height;
    // 必须为1
    bmiHeader.biPlanes = 1;
    // 像素所占位数
    bmiHeader.biBitCount = 32;
    // 0表示不压缩
    bmiHeader.biCompression = 0;
    // 位图数据大小
    bmiHeader.biSizeImage = bmpSize;
    // 水平分辨率(像素/米)
    bmiHeader.biXPelsPerMeter = 0;
    // 垂直分辨率(像素/米)
    bmiHeader.biYPelsPerMeter = 0;
    // 使用的颜色，0为使用全部颜色
    bmiHeader.biClrUsed = 0;
    // 重要的颜色数，0为所有颜色都重要
    bmiHeader.biClrImportant = 0;

    // 写位图信息头
    WriteFile(hFile, (LPSTR)&bmiHeader, sizeof(bmiHeader), &bytesWritten, NULL);
    // 写位图数据
    WriteFile(hFile, data, bmpSize, &bytesWritten, NULL);
    CloseHandle(hFile);
}

bool opDXGI::InitD3D11Device()
{
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
	D3D_FEATURE_LEVEL FeatureLevel;

	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			DriverTypes[DriverTypeIndex],
			nullptr, 0,
			FeatureLevels,
			NumFeatureLevels,
			D3D11_SDK_VERSION,
			&device_,
			&FeatureLevel,
			&deviceContext_);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (device_ == nullptr || deviceContext_ == nullptr)
	{
		return false;
	}

	return true;
}

bool opDXGI::InitDuplication()
{
	HRESULT hr = S_OK;

	IDXGIDevice* dxgiDevice = nullptr;
	hr = device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter* dxgiAdapter = nullptr;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	dxgiDevice->Release();
	if (FAILED(hr))
	{
		return false;
	}

	UINT output = 0;
	IDXGIOutput* dxgiOutput = nullptr;
	while (true)
	{
		hr = dxgiAdapter->EnumOutputs(output++, &dxgiOutput);
		if (hr == DXGI_ERROR_NOT_FOUND)
		{
			return false;
		}
		else
		{
			DXGI_OUTPUT_DESC desc;
			dxgiOutput->GetDesc(&desc);
			int width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
			int height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
			break;
		}
	}
	dxgiAdapter->Release();

	IDXGIOutput1* dxgiOutput1 = nullptr;
	hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));
	dxgiOutput->Release();
	if (FAILED(hr))
	{
		return false;
	}

	hr = dxgiOutput1->DuplicateOutput(device_, &duplication_);
	dxgiOutput1->Release();
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool opDXGI::GetDesktopFrame(ID3D11Texture2D** texture)
{
	HRESULT hr = S_OK;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	IDXGIResource* resource = nullptr;
	ID3D11Texture2D* acquireFrame = nullptr;
	*texture = nullptr;
	hr = duplication_->AcquireNextFrame(0, &frameInfo, &resource);
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&acquireFrame));
	resource->Release();
	if (FAILED(hr))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	acquireFrame->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	device_->CreateTexture2D(&desc, NULL, texture);
	if (texture && *texture)
	{
		deviceContext_->CopyResource(*texture, acquireFrame);
	}
	acquireFrame->Release();

	hr = duplication_->ReleaseFrame();
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void opDXGI::fmtFrameInfo(void* dst, HWND hwnd, int w, int h,bool inc) {
	m_frameInfo.hwnd = (unsigned __int64)hwnd;
	m_frameInfo.frameId = inc ? m_frameInfo.frameId + 1 : m_frameInfo.frameId;
	m_frameInfo.time = ::GetTickCount64();
	m_frameInfo.width = w;
	m_frameInfo.height = h;
	m_frameInfo.fmtChk();
	memcpy(dst, &m_frameInfo, sizeof(m_frameInfo));
}



