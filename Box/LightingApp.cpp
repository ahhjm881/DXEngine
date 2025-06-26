#include "LightingApp.h"


#include "DXEngine/Data/Color.h"
#include "DXEngine/Data/SphericalCoord.h"
#include "DXEngine/Common/GeometryGenerator.h"
#include "DXEngine/Data/Vertex.h"

#include <d3dcompiler.h>
#include <vector>

#include "DXEngine/Data/SubMesh.h"
#include <numbers>

using namespace DirectX;

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    const std::unique_ptr<EngineBase> engine = std::make_unique<LightingApp>();
    if (engine->Init(hInstance))
    {
        engine->Run();
    }

    return 0;
}

LightingApp::LightingApp()
{
    SetCameraSphericalCoord(5.f, -XM_PIDIV4, XM_PIDIV4);
    SetZoomSpeed(0.01f);

    XMStoreFloat4x4(&boxWorldMatrix, XMMatrixTranslation(0.0f, 0.0f, 0.0f));
    XMStoreFloat4x4(&sphereWorldMatrix, XMMatrixTranslation(3.0f, 0.0f, 0.0f));
    XMStoreFloat4x4(&cylinderWorldMatrix, XMMatrixTranslation(6.0f, 0.0f, 0.0f));

    constexpr float invSqrt3 = std::numbers::inv_sqrt3_v<float>;

    directionalLight =
    {
        .ambient = {1.0f, 1.0f, 1.0f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f},
        .direction = {invSqrt3, -invSqrt3, invSqrt3},
    };

    boxMaterial =
    {
        .ambient = {1.0f, 0.0f, 0.0f, 1.0f},
        .diffuse = {1.0f, 0.0f, 0.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f, 1.0f},
    };
}

LightingApp::~LightingApp() = default;

bool LightingApp::Init(HINSTANCE inInstanceHandle)
{
    if (!Super::Init(inInstanceHandle))
    {
        return false;
    }

    InitShaderResource();
    InitGeometry();
    return true;
}


struct CBufferPerObject
{
    XMFLOAT4X4 wvpMatrix;
    XMFLOAT4X4 worldInverseTransposeMatrix;
    XMFLOAT4X4 worldMatrix;

    Material material;
};

struct CBufferPerFrame
{
    alignas(16) XMFLOAT3 eyePosition;
    DirectionalLight directionalLight;
};

void LightingApp::InitShaderResource()
{
    ComPtr<ID3DBlob> vertexShaderBlob;
    ComPtr<ID3DBlob> vertexErrorShaderBlob;
    HRESULT hr = D3DCompileFromFile
    (
        L"Shader/box_vs.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS_Main",
        "vs_5_0",
        0, 0,
        &vertexShaderBlob,
        &vertexErrorShaderBlob
    );
    if (FAILED(hr))
    {
        if (vertexErrorShaderBlob)
        {
            OutputDebugStringA((char*)vertexErrorShaderBlob->GetBufferPointer());
            MessageBoxA(nullptr, (char*)vertexErrorShaderBlob->GetBufferPointer(), "Shader Compile Error", MB_OK);
        }
        else
        {
            MessageBoxA(nullptr, "Unknown error (no error blob returned)", "Shader Compile Error", MB_OK);
        }
        return;
    }

    ComPtr<ID3DBlob> pixelShaderBlob;
    ComPtr<ID3DBlob> pixelErrorShaderBlob;
    hr = D3DCompileFromFile
    (
        L"Shader/box_ps.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS_Main",
        "ps_5_0",
        0, 0,
        &pixelShaderBlob,
        &pixelErrorShaderBlob
        );
    if (FAILED(hr))
    {
        if (pixelErrorShaderBlob)
        {
            OutputDebugStringA((char*)pixelErrorShaderBlob->GetBufferPointer());
            MessageBoxA(nullptr, (char*)pixelErrorShaderBlob->GetBufferPointer(), "Shader Compile Error", MB_OK);
        }
        else
        {
            MessageBoxA(nullptr, "Unknown error (no error blob returned)", "Shader Compile Error", MB_OK);
        }
        return;
    }

    device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader
    );

    device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader
    );

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    device->CreateInputLayout(
        Vertex::PC::Desc.data(),
        static_cast<UINT>(Vertex::PC::Desc.size()),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        &inputLayout
    );

    immediateContext->IASetInputLayout(inputLayout.Get());
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    CD3D11_BUFFER_DESC perObjectCbufferDesc(sizeof(CBufferPerObject), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC,
                                            D3D11_CPU_ACCESS_WRITE);
    device->CreateBuffer(&perObjectCbufferDesc, nullptr, &cBufferPerObject);
    immediateContext->VSSetConstantBuffers(0, 1, cBufferPerObject.GetAddressOf());
    immediateContext->PSSetConstantBuffers(0, 1, cBufferPerObject.GetAddressOf());

    CD3D11_BUFFER_DESC perFrameCbufferDesc(sizeof(CBufferPerFrame), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC,
                                           D3D11_CPU_ACCESS_WRITE);
    device->CreateBuffer(&perFrameCbufferDesc, nullptr, &cBufferPerFrame);
    immediateContext->VSSetConstantBuffers(1, 1, cBufferPerFrame.GetAddressOf());
    immediateContext->PSSetConstantBuffers(1, 1, cBufferPerFrame.GetAddressOf());
}

void LightingApp::InitGeometry()
{
    const GeometryGenerator::MeshData sphereMeshData = GeometryGenerator::CreateGeodesicSphere(0.5f, 3);
    const GeometryGenerator::MeshData boxMeshData = GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f);
    const GeometryGenerator::MeshData cylinderMeshData = GeometryGenerator::CreateCylinder(0.5f, 0.5f, 2.0f, 5, 1);

    std::vector<Vertex::PC> vertices;
    vertices.reserve(
        sphereMeshData.vertices.size() +
        boxMeshData.vertices.size() +
        cylinderMeshData.vertices.size()
    );

    std::vector<UINT> indices;
    indices.reserve(
        sphereMeshData.indices.size() +
        boxMeshData.indices.size() +
        cylinderMeshData.indices.size()
    );

    std::vector<Vertex::PC> tempVertices;

    auto insertMeshData = [&](
        const GeometryGenerator::MeshData& meshData,
        Submesh& submesh
    )
    {
        tempVertices.resize(meshData.vertices.size());

        for (int i = 0; i < meshData.vertices.size(); ++i)
        {
            tempVertices[i].position = meshData.vertices[i].position;
            tempVertices[i].normal = meshData.vertices[i].normal;
        }

        submesh = Submesh(meshData.indices.size(), indices.size(), vertices.size());
        vertices.insert(vertices.end(), tempVertices.begin(), tempVertices.end());
        indices.insert(indices.end(), meshData.indices.begin(), meshData.indices.end());
    };

    insertMeshData(sphereMeshData, sphereSubmesh);
    insertMeshData(boxMeshData, boxSubmesh);
    insertMeshData(cylinderMeshData, cylinderSubmesh);

    const CD3D11_BUFFER_DESC vertexBufferDesc(
        static_cast<UINT>(sizeof(Vertex::PC) * vertices.size()),
        D3D11_BIND_VERTEX_BUFFER,
        D3D11_USAGE_IMMUTABLE
    );
    const D3D11_SUBRESOURCE_DATA vertexInitData
    {
        .pSysMem = vertices.data(),
    };
    device->CreateBuffer(&vertexBufferDesc, &vertexInitData, &vertexBuffer);

    const CD3D11_BUFFER_DESC indexBufferDesc(
        static_cast<UINT>(sizeof(UINT) * indices.size()),
        D3D11_BIND_INDEX_BUFFER,
        D3D11_USAGE_IMMUTABLE
    );
    const D3D11_SUBRESOURCE_DATA indexInitData
    {
        .pSysMem = indices.data(),
    };
    device->CreateBuffer(&indexBufferDesc, &indexInitData, &indexBuffer);

    constexpr UINT strideSize[]{sizeof(Vertex::PC)};
    constexpr UINT offsets[]{0};

    immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), strideSize, offsets);
    immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void LightingApp::OnResize()
{
    Super::OnResize();
    XMStoreFloat4x4(&projectionMatrix,
                    XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), GetAspectRatio(), 1.0f, 10000.0f));
}

void LightingApp::Update(float deltaSeconds)
{
    const XMVECTOR cameraPosition = SphericalCoord::SphericalToCartesian(GetCameraSphericalCoord());
    const XMVECTOR focusPosition = XMVectorZero();
    const XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMStoreFloat4x4(&viewMatrix, XMMatrixLookAtLH(cameraPosition, focusPosition, upDirection));
    XMStoreFloat3(&eyePosition, cameraPosition);
}

void LightingApp::Render()
{
    immediateContext->ClearRenderTargetView(renderTargetView.Get(), LinearColors::LightSteelBlue);
    immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    D3D11_MAPPED_SUBRESOURCE subresource;
    immediateContext->Map(cBufferPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    CBufferPerFrame perFrame;
    perFrame.eyePosition = eyePosition;
    perFrame.directionalLight = directionalLight;
    *static_cast<CBufferPerFrame*>(subresource.pData) = perFrame;
    immediateContext->Unmap(cBufferPerFrame.Get(), 0);

    RenderObject(sphereSubmesh, sphereWorldMatrix);
    RenderObject(boxSubmesh, boxWorldMatrix);
    RenderObject(cylinderSubmesh, cylinderWorldMatrix);

    swapChain->Present(0, 0);
}


// ReSharper disable once CppMemberFunctionMayBeConst
void LightingApp::RenderObject(const Submesh& submesh, const XMFLOAT4X4& worldMatrix)
{
    const XMMATRIX wvpMatrix = XMLoadFloat4x4(&worldMatrix) * XMLoadFloat4x4(&viewMatrix) * XMLoadFloat4x4(
        &projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE subresource;
    immediateContext->Map(cBufferPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    CBufferPerObject perObject{};
    XMStoreFloat4x4(&perObject.wvpMatrix, wvpMatrix);
    perObject.worldMatrix = worldMatrix;
    XMStoreFloat4x4(&perObject.worldInverseTransposeMatrix,XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&worldMatrix))));
    perObject.material = boxMaterial;
    *static_cast<CBufferPerObject*>(subresource.pData) = perObject;
    immediateContext->Unmap(cBufferPerObject.Get(), 0);

    immediateContext->DrawIndexed(submesh.indexCount, submesh.startIndexLocation, submesh.baseVertexLocation);
}
