#pragma once

#pragma comment(lib, "d3d9.lib")

#include <d3d9.h>
#include <windows.h>

#include <memory>

namespace imgui_app {

    struct DeviceD3D {
    LPDIRECT3D9 d3d = nullptr;
    LPDIRECT3DDEVICE9 d3d_device = nullptr;
    D3DPRESENT_PARAMETERS d3d_present_parameters = {};
};

struct DeviceD3DDeleter {
    void operator()(DeviceD3D* dev) const;
};

std::unique_ptr<DeviceD3D, DeviceD3DDeleter> CreateDeviceD3D(
    HWND window_handle
);
void ResetDevice(DeviceD3D* dev);
HRESULT RenderAndPresent(DeviceD3D* dev);

}  // namespace imgui_app