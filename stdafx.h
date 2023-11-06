#pragma once

#define NOMINMAX

#include <atlbase.h>
#include <atltypes.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlcrack.h>
#include <atlexcept.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlstr.h>

#include <d3d11.h>
#include <d3d11_1.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#include <wrl.h>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Wrappers;

using Microsoft::WRL::ComPtr;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "runtimeobject.lib")
