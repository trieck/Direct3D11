#pragma once
#include "DevResources.h"

class Direct3DApp : public CAppModule
{
public:
    HRESULT Init(_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = nullptr);
    void Term();

    DevResources& devResources();

private:
    DevResources m_devResources;
};
