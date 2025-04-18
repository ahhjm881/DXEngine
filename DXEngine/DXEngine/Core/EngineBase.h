#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

class EngineBase
{
public:
    void Init(HINSTANCE hInst);
    void OnResize();

protected:
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> immediateContext;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    ComPtr<ID3D11DepthStencilView> depthStencilView;
    UINT msaaQuality = 0;

    HINSTANCE instanceHandle{};
    HWND windowHandle{};
    UINT clientWidth = 1024;
    UINT clientHeight = 1024;
    std::wstring windowTitle ;
    std::wstring windowClassName ;

private:
    void InitWindow();
    void InitDX();

};
