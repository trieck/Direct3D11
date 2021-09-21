#pragma once

/////////////////////////////////////////////////////////////////////////////
class ResourceLoader
{
public:
    HRESULT Load(HINSTANCE hInstance, INT nResourceID, LPCWSTR type);

    LPCBYTE data() const;
    size_t size() const;

private:
    LPBYTE m_pData = nullptr;
    size_t m_size = 0;
};
