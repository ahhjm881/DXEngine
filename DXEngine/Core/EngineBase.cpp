#include "EngineBase.h"

namespace
{
    LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void EngineBase::Init(HINSTANCE hInst)
{
    instanceHandle = hInst;
    windowTitle = TEXT("SDAS");
    windowClassName = TEXT("SDAaS");

    InitWindow();
    InitDX();
}

void EngineBase::InitWindow()
{
    // 윈도우 클래스 정의 및 등록
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instanceHandle;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = windowClassName.c_str();
    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"클래스 등록 실패", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // 창의 UI를 포함하지 않은 클라이언트 영역의 사이즈가 clientWidth, clientHeight가 되도록 보정
    RECT rect = {0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight)};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_APPWINDOW);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    // 윈도우를 스크린 중심에 띄우기 위한 오프셋
    const int centerX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    const int centerY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    // 윈도우 생성
    windowHandle = CreateWindowEx(WS_EX_APPWINDOW, windowClassName.c_str(), windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                                  centerX, centerY, width, height, nullptr, nullptr, instanceHandle, nullptr);
    if (!windowHandle)
    {
        MessageBox(nullptr, L"윈도우 생성 실패", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // 윈도우 표시
    ShowWindow(windowHandle, SW_SHOW);
    UpdateWindow(windowHandle);
}

void EngineBase::InitDX()
{
    D3D_FEATURE_LEVEL featureLevel;
    UINT createDeviceFlag = 0;

#ifdef _DEBUG
    createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlag,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device,
        &featureLevel,
        &immediateContext
    );

    result = device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        4,
        &msaaQuality
    );

    DXGI_SWAP_CHAIN_DESC swapChainDesc
    {
        .BufferDesc = DXGI_MODE_DESC
        {
            .Width = clientWidth,
            .Height = clientHeight,
            .RefreshRate = DXGI_RATIONAL
            {
                .Numerator = 60,
                .Denominator = 1,
            },
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
            .Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
        },

        .SampleDesc = DXGI_SAMPLE_DESC
        {
            .Count = 4,
            .Quality = msaaQuality - 1,
        },

        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 1,
        .OutputWindow = windowHandle,
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .Flags = 0
    };

    ComPtr<IDXGIDevice> dxgiDevice;
    device->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);

    ComPtr<IDXGIAdapter> dxgiApdater;
    dxgiDevice->GetAdapter(&dxgiApdater);

    ComPtr<IDXGIFactory> dxgiFactory;
    dxgiApdater->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);

    dxgiFactory->CreateSwapChain(device.Get(), &swapChainDesc, &swapChain);
}

void EngineBase::OnResize()
{
    if (!device || !immediateContext || !swapChain)
    {
        return;
    }

    immediateContext->OMSetRenderTargets(0, nullptr, nullptr);
    renderTargetView.Reset();
    depthStencilView.Reset();

    HRESULT result = swapChain->ResizeBuffers(1, clientWidth, clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    ComPtr<ID3D11Texture2D> backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);

    device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

    D3D11_TEXTURE2D_DESC depthStencilDesc
    {
        .Width = clientWidth,
        .Height = clientHeight,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,

        .SampleDesc = DXGI_SAMPLE_DESC
        {
            .Count = 4,
            .Quality = msaaQuality - 1,
        },

        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        .CPUAccessFlags = 0,
        .MiscFlags = 0
    };

    ComPtr<ID3D11Texture2D> depthStencilBuffer;
    device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);

    immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    D3D11_VIEWPORT viewPort
    {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = static_cast<FLOAT>(clientWidth),
        .Height = static_cast<FLOAT>(clientHeight),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    };

    immediateContext->RSSetViewports(1, &viewPort);
}
