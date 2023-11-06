#pragma once

/////////////////////////////////////////////////////////////////////////////
class ResourceLoader
{
public:
    static HRESULT Load(HINSTANCE hInstance, INT nResourceID, LPCWSTR type,
                        LPCBYTE& pdata, UINT& size);
    static HRESULT Load(HINSTANCE hInstance, LPCWSTR resource, LPCWSTR type,
        LPCBYTE& pdata, UINT& size);
};
