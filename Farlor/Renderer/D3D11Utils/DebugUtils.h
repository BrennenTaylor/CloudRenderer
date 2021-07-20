#pragma once

#include <d3d11.h>

namespace Farlor
{
    class D3D11DebugUtils
    {
    public:
        static void SetDebugName(ID3D11DeviceChild* pDeviceChild, const std::string& name)
        {
            pDeviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(name.size()), name.c_str());
        }
    };
}