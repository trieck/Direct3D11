#pragma once

/////////////////////////////////////////////////////////////////////////////
class TextureLoader
{
public:
    HRESULT Load(HINSTANCE hInstance, INT nResourceID);

    LPCBYTE dds() const;
    size_t size() const;

private:
    LPBYTE m_pDDS = nullptr;
    size_t m_size = 0;
};
