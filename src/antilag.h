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
            AMD::AntiLag2DX12::Update(&context_dx12, true, max_fps);
        else
            AMD::AntiLag2DX11::Update(&context_dx11, true, max_fps);
    }

    void mark_end_of_rendering() {
        if (context_dx12.m_pAntiLagAPI)
            AMD::AntiLag2DX12::MarkEndOfFrameRendering(&context_dx12);
    }

    void present_start(bool interpolated_frame) {
        if (context_dx12.m_pAntiLagAPI)
            AMD::AntiLag2DX12::SetFrameGenFrameType(&context_dx12, interpolated_frame);
    }

    void set_fps_limit(unsigned int fps) {
        log(std::format("Max fps: {}", fps));
        max_fps = fps;
    }
};