#include "stdafx.h"
#include "Model.h"
#include "Direct3DApp.h"
#include "ResourceLoader.h"

extern Direct3DApp _Module;

Model::~Model()
{
    m_indexBuffer.Reset();
    m_vertexBuffer.Reset();
}

HRESULT Model::Load(HINSTANCE hInstance, INT nModelID, LPCWSTR modelType, INT nTextureID, LPCWSTR textureType)
{
    LPCBYTE pModel;
    UINT modelSize;

    auto& devResources = _Module.devResources();

    auto hr = ResourceLoader::Load(hInstance, nModelID, modelType, pModel, modelSize);
    if (FAILED(hr)) {
        return hr;
    }

    hr = ParseModel(pModel, modelSize);
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(m_vertices.data(), 
        static_cast<UINT>(m_vertices.size()) * sizeof(Vertex),
        D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    hr = devResources.CreateBuffer(m_indices.data(), 
        static_cast<UINT>(m_indices.size()) * sizeof(ULONG),
        D3D11_BIND_INDEX_BUFFER,
        D3D11_USAGE_DEFAULT, 0, m_indexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

void Model::Render(ID3D11DeviceContext1* pContext)
{
    ATLASSERT(pContext);

    // set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    // set the index buffer
    pContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    // set the primitive topology
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw vertices, starting from vertex 0
    pContext->DrawIndexed(static_cast<UINT>(m_indices.size()), 0, 0);
}

HRESULT Model::ParseModel(LPCBYTE pdata, UINT size)
{
    std::string input(reinterpret_cast<LPCSTR>(pdata), size);

    std::istringstream ss(input);

    ULONG i = 0;
    while (!ss.eof()) {
        Vertex vertex{};

        ss >> vertex.position.x
            >> vertex.position.y
            >> vertex.position.z
            >> vertex.texture.x
            >> vertex.texture.y
            >> vertex.normal.x
            >> vertex.normal.y
            >> vertex.normal.z;

        if (ss.fail()) {
            break;
        }

        m_vertices.emplace_back(std::move(vertex));
        m_indices.emplace_back(i++);
    }
    
    return S_OK;
}
