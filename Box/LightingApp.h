#pragma once
#include <Windows.h>
#include "DXEngine/Core/SphericalCamera.h"

class BasicShaderPass;

class LightingApp final : public SphericalCamera
{
    DECLARE_ENGINE(LightingApp, SphericalCamera)

public:
    LightingApp();
    ~LightingApp() override;

    bool Init(HINSTANCE inInstanceHandle) override;
    void InitShaderResource();
    void InitGeomery();
    void OnResize() override;

protected:
    virtual void Update(float deltaSeconds) override;
    virtual void Render() override;
private:
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT3 eyePosition;

    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> cBuffer;

    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    UINT indicesCount;
};
