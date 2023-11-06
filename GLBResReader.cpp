#include "stdafx.h"
#include "GLBResReader.h"
#include "ResourceLoader.h"

#include <GLTFSDK/GLBResourceReader.h>

using namespace Microsoft::glTF;
using namespace GLBResReader;

GLBResourceReaderPtr GLBResReader::LoadReader(HINSTANCE hInstance, INT nResourceID, LPCWSTR type)
{
    LPCBYTE data;
    UINT size;

    auto hr = ResourceLoader::Load(hInstance, nResourceID, type, data, size);
    if (FAILED(hr)) {
        throw InvalidGLTFException("Cannot find resource");
    }

    std::string input(reinterpret_cast<LPCSTR>(data), size);
    auto stream = std::make_shared<std::istringstream>(std::move(input));

    auto resourceReader = std::make_unique<GLBResourceReader>(static_cast<std::shared_ptr<IStreamReader>>(nullptr),
                                                              stream);

    return resourceReader;
}
