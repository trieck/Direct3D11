#pragma once

#include <GLTFSDK/Document.h>
#include <GLTFSDK/GLBResourceReader.h>

namespace GLBResReader
{
    using GLBResourceReaderPtr = std::unique_ptr<Microsoft::glTF::GLBResourceReader>;

    GLBResourceReaderPtr LoadReader(HINSTANCE hInstance, INT nResourceID, LPCWSTR type);
};
