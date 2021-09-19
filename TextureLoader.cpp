#include "stdafx.h"
#include "TextureLoader.h"

/////////////////////////////////////////////////////////////////////////////
HRESULT TextureLoader::Load(HINSTANCE hInstance, INT nResourceID)
{
    ATLASSERT(hInstance);

    m_pDDS = nullptr;
    m_size = 0;

    auto hResInfo = FindResource(hInstance, MAKEINTRESOURCE(nResourceID), L"DDS");
    if (hResInfo == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_size = SizeofResource(hInstance, hResInfo);

    auto hResData = LoadResource(hInstance, hResInfo);
    if (hResData == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_pDDS = static_cast<LPBYTE>(LockResource(hResData));
    if (m_pDDS == nullptr) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

LPCBYTE TextureLoader::dds() const
{
    return m_pDDS;
}

size_t TextureLoader::size() const
{
    return m_size;
}
