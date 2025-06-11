#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <iostream>

typedef HRESULT(STDMETHODCALLTYPE* PFN_GetDesc1)(IDXGIAdapter1*, DXGI_ADAPTER_DESC1*);
PFN_GetDesc1 o_GetDesc1 = nullptr;
static bool spoof_intel = false;

HRESULT STDMETHODCALLTYPE hk_GetDesc1(IDXGIAdapter1* pThis, DXGI_ADAPTER_DESC1* pDesc) {
    spdlog::debug("GetDesc1 called");

    HRESULT hr = o_GetDesc1(pThis, pDesc);

    if (SUCCEEDED(hr) && spoof_intel) {
        spdlog::debug("GetDesc1 spoofing Intel");

        pDesc->VendorId = 0x8086;
        pDesc->DeviceId = 0x56A0;

        std::wstring szName = L"Intel(R) Arc(TM) A770 Graphics";
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());
    }

    return hr;
}

void hook_GetDesc1() {
    if (o_GetDesc1)
        return;

    IDXGIFactory1* pFactory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
        return;

    IDXGIAdapter1* pAdapter = nullptr;
    if (FAILED(pFactory->EnumAdapters1(0, &pAdapter))) {
        pFactory->Release();
        return;
    }

    void** vtable = *(void***)pAdapter;

    DWORD oldProtect;
    VirtualProtect(&vtable[10], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);

    o_GetDesc1 = (PFN_GetDesc1)vtable[10];
    vtable[10] = (void*)&hk_GetDesc1;

    VirtualProtect(&vtable[10], sizeof(void*), oldProtect, &oldProtect);

    pAdapter->Release();
    pFactory->Release();
}

void unhook_GetDesc1() {
    if (!o_GetDesc1)
        return;

    IDXGIFactory1* pFactory = nullptr;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
        return;

    IDXGIAdapter1* pAdapter = nullptr;
    if (FAILED(pFactory->EnumAdapters1(0, &pAdapter))) {
        pFactory->Release();
        return;
    }

    void** vtable = *(void***)pAdapter;

    DWORD oldProtect;
    VirtualProtect(&vtable[10], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);

    // hk_GetDesc1 will still get called if hooks were made after we made ours
    vtable[10] = o_GetDesc1;

    VirtualProtect(&vtable[10], sizeof(void*), oldProtect, &oldProtect);

    pAdapter->Release();
    pFactory->Release();
}

void spoof(bool toggle) {
    spoof_intel = toggle;
}