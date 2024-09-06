#pragma once

#include <dxgi.h>
#if defined __MINGW64__ || defined __MINGW32__
#include "../include/d3d12.h"
#else
#include <d3d12.h>
#endif

#include "../include/ffx_antilag2_dx12.h"
#include "../include/ffx_antilag2_dx11.h"

#include "log.h"

class AntiLag {
    AMD::AntiLag2DX12::Context context_dx12 = {};
    AMD::AntiLag2DX11::Context context_dx11 = {};
    unsigned int max_fps = 0;
    bool al_available = false;

public:
    bool calls_input_sample = false;
    bool calls_sleep = false;

    inline HRESULT init(IUnknown *pDevice) {
        if (!context_dx12.m_pAntiLagAPI && !context_dx11.m_pAntiLagAPI) {
            ID3D12Device* device = nullptr;
            HRESULT hr = pDevice->QueryInterface(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
            HRESULT init_return = S_FALSE;
            if (hr == S_OK) {
                init_return = AMD::AntiLag2DX12::Initialize(&context_dx12, device);
            } else {
                init_return = AMD::AntiLag2DX11::Initialize(&context_dx11);
            }
            al_available = init_return == S_OK;
        }
        return 0x2137;
    }

    inline HRESULT update() {
        if (!al_available) return S_FALSE;
        if (context_dx12.m_pAntiLagAPI)
            return AMD::AntiLag2DX12::Update(&context_dx12, true, max_fps);
        else
            return AMD::AntiLag2DX11::Update(&context_dx11, true, max_fps);
    }

    void set_fps_limit(unsigned int fps) {
        log(std::format("Max fps: {}", fps));
        max_fps = fps;
    }
};