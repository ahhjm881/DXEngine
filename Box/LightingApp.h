#pragma once
#include <Windows.h>
#include "DXEngine/Core/SphericalCamera.h"
#include "DXEngine/Data/SubMesh.h"
#include "Light.h"


class BasicShaderPass;

class LightingApp final : public SphericalCamera
{
    DECLARE_ENGINE(LightingApp, SphericalCamera)

public:
    LightingApp();
    ~LightingApp() override;

    bool Init(HINSTANCE inInstanceHandle) override;
    void InitShaderResource();
    void InitGeometry();
    void OnResize() override;

protected:
    void Update(float deltaSeconds) override;
    void Render() override;

private:
    void RenderObject(const Submesh& submesh, const DirectX::XMFLOAT4X4& worldMatrix);
    
private:
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
    DirectX::XMFLOAT3 eyePosition;

    DirectionalLight directionalLight;
    


    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    
    ComPtr<ID3D11Buffer> cBufferPerObject;
    ComPtr<ID3D11Buffer> cBufferPerFrame;

    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    Submesh sphereSubmesh{};
    DirectX::XMFLOAT4X4 sphereWorldMatrix;
    Material sphereMaterial;
    Submesh boxSubmesh{};
    DirectX::XMFLOAT4X4 boxWorldMatrix;
    Material boxMaterial;
    Submesh cylinderSubmesh{};
    DirectX::XMFLOAT4X4 cylinderWorldMatrix;
    Material cylinderMaterial;
};
