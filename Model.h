#pragma once
#include <GLTFSDK/GLBResourceReader.h>

class Model
{
public:
    Model() = default;
    ~Model() = default;

    HRESULT Load(HINSTANCE hInstance, INT nModelID, LPCWSTR modelType);
    void Render(ID3D11DeviceContext1* pContext);

private:
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texture;
        XMFLOAT3 normal;
    };

    HRESULT LoadModel(const Microsoft::glTF::GLBResourceReader& reader);

    HRESULT LoadScene(const Microsoft::glTF::GLBResourceReader& reader,
                      const Microsoft::glTF::Document& doc,
                      const Microsoft::glTF::Scene& scene);

    HRESULT LoadSceneNode(const Microsoft::glTF::GLBResourceReader& reader,
                          const Microsoft::glTF::Document& doc,
                          const Microsoft::glTF::Node& node,
                          INT parentID = -1);

    HRESULT LoadMeshNode(const Microsoft::glTF::GLBResourceReader& reader,
                         const Microsoft::glTF::Document& doc,
                         const Microsoft::glTF::Node& node);
    HRESULT LoadMaterialNode(const Microsoft::glTF::GLBResourceReader& reader,
                             const Microsoft::glTF::Document& doc,
                             const Microsoft::glTF::Material& material);
    HRESULT LoadBuffer(const Microsoft::glTF::GLBResourceReader& reader,
                       const Microsoft::glTF::Document& doc,
                       const std::string& accessorID,
                       const char* type);
    HRESULT LoadBuffer(const Microsoft::glTF::GLBResourceReader& reader,
                       const Microsoft::glTF::Document& doc,
                       const Microsoft::glTF::MeshPrimitive& primitive,
                       const char* name);
    HRESULT LoadBuffer(const Microsoft::glTF::GLBResourceReader& reader,
                       const Microsoft::glTF::Document& doc,
                       const Microsoft::glTF::BufferView& view,
                       const Microsoft::glTF::Accessor& accessor,
                       const char* type);


    void LoadTransform(const Microsoft::glTF::Node& node);

    ComPtr<ID3D11Buffer> m_matrixBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11ShaderResourceView> m_textureView;

    XMMATRIX m_modelMatrix{};
    std::vector<Vertex> m_vertices{};
    std::vector<ULONG> m_indices{};
};
