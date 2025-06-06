#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>
#include <iostream>

typedef HRESULT(STDMETHODCALLTYPE* GetDesc1Func)(IDXGIAdapter1*, DXGI_ADAPTER_DESC1*);
GetDesc1Func g_originalGetDesc1 = nullptr;
static bool spoof_intel = false;

// Our hook
HRESULT STDMETHODCALLTYPE MyGetDesc1Hook(IDXGIAdapter1* pThis, DXGI_ADAPTER_DESC1* pDesc) {
    std::cout << "IDXGIAdapter1::GetDesc1 hooked!" << std::endl;

    // Call the original
    HRESULT hr = g_originalGetDesc1(pThis, pDesc);

    // Modify the description as an example
    if (SUCCEEDED(hr) && spoof_intel) {
        pDesc->VendorId = 0x8086;
        pDesc->DeviceId = 0x56A0;

        std::wstring szName = L"Intel(R) Arc(TM) A770 Graphics";
        std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
        std::wcscpy(pDesc->Description, szName.c_str());
    }

    return hr;
}

void HookIDXGIAdapter1_GetDesc1() {
    if (g_originalGetDesc1)
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

    g_originalGetDesc1 = (GetDesc1Func)vtable[10];  // Index 10 is GetDesc1 in IDXGIAdapter1 vtable
    vtable[10] = (void*)&MyGetDesc1Hook;

    VirtualProtect(&vtable[10], sizeof(void*), oldProtect, &oldProtect);

    pAdapter->Release();
    pFactory->Release();
}

void spoof(bool toggle) {
    spoof_intel = toggle;
}