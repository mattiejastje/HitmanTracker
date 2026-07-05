#pragma once

#pragma comment(lib, "d3d9.lib")

#include <d3d9.h>
#include <windows.h>

#include <memory>

namespace imgui_app {

struct DeviceD3D {
    struct State {
        D3DPRESENT_PARAMETERS present_parameters = {};
        bool is_lost = false;
    };

    LPDIRECT3D9 d3d = nullptr;
    LPDIRECT3DDEVICE9 d3d_device = nullptr;
    State state{};
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