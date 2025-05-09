#pragma once

#pragma comment(lib, "d3d9.lib")

#include <d3d9.h>
#include <windows.h>

#include <memory>

struct DeviceD3D {
    LPDIRECT3D9 g_pD3D = nullptr;
    LPDIRECT3DDEVICE9 g_pd3dDevice = nullptr;
    D3DPRESENT_PARAMETERS g_d3dpp = {};
};

struct DeviceD3DDeleter {
    void operator()(DeviceD3D* dev) const;
};

std::unique_ptr<DeviceD3D, DeviceD3DDeleter> CreateDeviceD3D(HWND window_handle
);
