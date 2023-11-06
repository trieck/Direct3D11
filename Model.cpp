#include "stdafx.h"
#include "Model.h"
#include "Direct3DApp.h"
#include "GLBResReader.h"

#include <GLTFSDK/Deserialize.h>

using namespace Microsoft::glTF;

static XMVECTOR LoadFloat3(const Vector3& v3);
static XMVECTOR LoadFloat4(const Quaternion& q);

struct alignas(16) MatrixBuffer
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX projection;
};

extern Direct3DApp _Module;

//HRESULT Model::Load(HINSTANCE hInstance, INT nModelID, LPCWSTR modelType, INT nTextureID, LPCWSTR textureType)
//{
//    LPCBYTE pModel;
//    UINT modelSize;
//
//    auto& devResources = _Module.devResources();
//
//    auto hr = ResourceLoader::Load(hInstance, nModelID, modelType, pModel, modelSize);
//    if (FAILED(hr)) {
//        return hr;
//    }
//
//    hr = ParseModel(pModel, modelSize);
//    if (FAILED(hr)) {
//        return hr;
//    }
//
//    hr = devResources.CreateBuffer(m_vertices.data(),
//                                   static_cast<UINT>(m_vertices.size()) * sizeof(Vertex),
//                                   D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DEFAULT, 0, m_vertexBuffer.GetAddressOf());
//    if (FAILED(hr)) {
//        return hr;
//    }
//
//    hr = devResources.CreateBuffer(m_indices.data(),
//                                   static_cast<UINT>(m_indices.size()) * sizeof(ULONG),
//                                   D3D11_BIND_INDEX_BUFFER,
//                                   D3D11_USAGE_DEFAULT, 0, m_indexBuffer.GetAddressOf());
//    if (FAILED(hr)) {
//        return hr;
//    }
//
//    hr = devResources.LoadTextureFromResource(hInstance, nTextureID,
//                                              textureType, m_textureView.GetAddressOf());
//    if (FAILED(hr)) {
//        return hr;
//    }
//
//    return S_OK;
//}

HRESULT Model::Load(HINSTANCE hInstance, INT nModelID, LPCWSTR modelType)
{
    auto& devResources = _Module.devResources();

    auto hr = devResources.CreateBuffer(sizeof(MatrixBuffer),
                                        D3D11_BIND_CONSTANT_BUFFER,
                                        D3D11_USAGE_DYNAMIC,
                                        D3D11_CPU_ACCESS_WRITE,
                                        m_matrixBuffer.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    try {
        auto reader = GLBResReader::LoadReader(hInstance, nModelID, modelType);

        hr = LoadModel(*reader);
        if (FAILED(hr)) {
            return hr;
        }

    } catch (const GLTFException& err) {
        ATLTRACE(err.what());
        return E_FAIL;
    }

    return hr;
}

HRESULT Model::LoadModel(const GLBResourceReader& reader)
{
    const auto& json = reader.GetJson();

    auto doc = Deserialize(json);

    auto hr = LoadScene(reader, doc, doc.GetDefaultScene());

    return hr;
}

HRESULT Model::LoadScene(const GLBResourceReader& reader, const Document& doc, const Scene& scene)
{
    for (const auto& nodeName : scene.nodes) {
        const auto& node = doc.nodes[nodeName];
        auto hr = LoadSceneNode(reader, doc, node);
        if (FAILED(hr)) {
            return hr;
        }
    }

    return S_OK;
}

HRESULT Model::LoadSceneNode(const GLBResourceReader& reader, const Document& doc,
                             const Node& node, INT parentID)
{
    auto name = node.name.length() > 0 ? node.name : "node-" + node.id;

    if (!node.meshId.empty()) {
        auto hr = LoadMeshNode(reader, doc, node);
        if (FAILED(hr)) {
            return hr;
        }
    }

    return S_OK;
}

HRESULT Model::LoadMeshNode(const GLBResourceReader& reader, const Document& doc, const Node& node)
{
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxAnisotropy = 1;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    auto& devResources = _Module.devResources();

    auto hr = devResources.CreateSamplerState(&sd, m_samplerState.GetAddressOf());
    if (FAILED(hr)) {
        return hr;
    }

    LoadTransform(node);

    const auto& meshNode = doc.meshes[node.meshId];
    for (const auto& primitive : meshNode.primitives) {
        const auto& material = doc.materials[primitive.materialId];
        hr = LoadMaterialNode(reader, doc, material);
        if (FAILED(hr)) {
            return hr;
        }

        hr = LoadBuffer(reader, doc, primitive.indicesAccessorId, "INDICES");
        if (FAILED(hr)) {
            return hr;
        }

        hr = LoadBuffer(reader, doc, primitive, "POSITION");
        if (FAILED(hr)) {
            return hr;
        }

        hr = LoadBuffer(reader, doc, primitive, "NORMAL");
        if (FAILED(hr)) {
            return hr;
        }

        hr = LoadBuffer(reader, doc, primitive, "TANGENT");
        if (FAILED(hr)) {
            return hr;
        }

        hr = LoadBuffer(reader, doc, primitive, "TEXCOORD_0");
        if (FAILED(hr)) {
            return hr;
        }
    }

    return S_OK;
}

HRESULT Model::LoadMaterialNode(const GLBResourceReader& reader, const Document& doc, const Material& material)
{
    /*if (!material.metallicRoughness.baseColorTexture.textureId.empty()) {
        ATLASSERT(0);
    }

    if (!material.metallicRoughness.metallicRoughnessTexture.textureId.empty()) {
        ATLASSERT(0);
    }

    if (!material.emissiveTexture.textureId.empty()) {
        ATLASSERT(0);
    }

    if (!material.occlusionTexture.textureId.empty()) {
        ATLASSERT(0);
    }

    if (!material.normalTexture.textureId.empty()) {
        ATLASSERT(0);
    }*/

    return S_OK;
}

void Model::LoadTransform(const Node& node)
{
    auto type = node.GetTransformationType();
    if (type == TRANSFORMATION_MATRIX) {
        ATLASSERT(0);
    } else {
        constexpr XMFLOAT3 emptyVector = { 0, 0, 0 };

        auto scale = LoadFloat3(node.scale);
        auto origin = XMLoadFloat3(&emptyVector);
        auto rotation = LoadFloat4(node.rotation);
        auto translation = LoadFloat3(node.translation);

        auto matrix = XMMatrixAffineTransformation(
            scale,
            origin,
            rotation,
            translation);

        m_modelMatrix = XMMatrixTranspose(matrix);
    }
}

HRESULT Model::LoadBuffer(const GLBResourceReader& reader, const Document& doc, const std::string& accessorID,
                          const char* type)
{
    if (accessorID.empty() || std::stoi(accessorID.c_str()) < 0) {
        return S_OK;
    }

    auto indicesAcc = doc.accessors[accessorID];
    auto bufferView = doc.bufferViews[indicesAcc.bufferViewId];

    return LoadBuffer(reader, doc, bufferView, indicesAcc, type);
}

HRESULT Model::LoadBuffer(const GLBResourceReader& reader, const Document& doc, const BufferView& view,
                          const Accessor& accessor, const char* type)
{
    auto& devResources = _Module.devResources();

    auto bufferData = reader.ReadBinaryData<char>(doc, view);
    auto accId = std::stoi(accessor.id);

    auto* pSysMem = bufferData.data() + accessor.byteOffset;
    auto byteWidth = view.byteLength - accessor.byteOffset;

    //return S_OK;
    return E_FAIL;
}

HRESULT Model::LoadBuffer(const GLBResourceReader& reader, const Document& doc, const MeshPrimitive& primitive,
                          const char* name)
{
    if (!primitive.HasAttribute(name)) {
        return S_OK;
    }

    auto accessorID = primitive.GetAttributeAccessorId(name);

    return LoadBuffer(reader, doc, accessorID, name);
}

void Model::Render(ID3D11DeviceContext1* pContext)
{
    ATLASSERT(pContext);

    pContext->PSSetShaderResources(0, 1, m_textureView.GetAddressOf());

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

XMVECTOR LoadFloat3(const Vector3& v3)
{
    XMFLOAT3 f3{ v3.x, v3.y, v3.z };

    return XMLoadFloat3(&f3);
}

XMVECTOR LoadFloat4(const Quaternion& q)
{
    XMFLOAT4 f4{ q.x, q.y, q.z, q.w };

    return XMLoadFloat4(&f4);
}
