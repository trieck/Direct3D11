#pragma once

class Model
{
public:
    Model() = default;
    ~Model();

    HRESULT Load(HINSTANCE hInstance, INT nModelID, LPCWSTR modelType,
                 INT nTextureID, LPCWSTR textureType);
    void Render(ID3D11DeviceContext1* pContext);

private:
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
        XMFLOAT3 normal;
    };

    HRESULT ParseModel(LPCBYTE pdata, UINT size);

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    std::vector<Vertex> m_vertices;
    std::vector<ULONG> m_indices;
};

