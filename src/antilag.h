#pragma once

#include <dxgi.h>
#if defined __MINGW64__ || defined __MINGW32__
#include "../include/d3d12.h"
#else
#include <d3d12.h>
#endif

#include "../include/ffx_antilag2_dx12.h"
#include "../include/ffx_antilag2_dx11.h"

class AntiLag {
    AMD::AntiLag2DX12::Context context_dx12 = {};
    AMD::AntiLag2DX11::Context context_dx11 = {};

    void init_dx12(ID3D12Device *pDevice) {
        auto res = AMD::AntiLag2DX12::Initialize(&context_dx12, pDevice);
    }
    void init_dx11() {
        auto res = AMD::AntiLag2DX11::Initialize(&context_dx11);
    }

public:
    void init(IUnknown *pDevice) {
        if (!context_dx12.m_pAntiLagAPI && !context_dx11.m_pAntiLagAPI) {
            ID3D12Device* device = nullptr;
            HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
            if (hr == S_OK) {
                init_dx12(device);
            } else {
                init_dx11();
            }
        }
    }

    void update() {
        if (context_dx12.m_pAntiLagAPI)
            AMD::AntiLag2DX12::Update(&context_dx12, true, 0);
        else
            AMD::AntiLag2DX11::Update(&context_dx11, true, 0);
    }
};