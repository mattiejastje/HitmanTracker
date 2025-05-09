#include "deviced3d.hpp"

#include <imgui_impl_dx9.h>

#include "../logging.hpp"

void DeviceD3DDeleter::operator()(DeviceD3D* dev) const {
    logging::debug("Releasing Direct3D...");
    if (dev->d3d_device) {
        dev->d3d_device->Release();
    }
    if (dev->d3d) {
        dev->d3d->Release();
    }
}

std::unique_ptr<DeviceD3D, DeviceD3DDeleter> CreateDeviceD3D(HWND window_handle
) {
    logging::debug("Initializing Direct3D...");
    auto dev = std::unique_ptr<DeviceD3D, DeviceD3DDeleter>(new DeviceD3D);
    dev->d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!dev->d3d) return nullptr;
    ZeroMemory(
        &dev->d3d_present_parameters, sizeof(dev->d3d_present_parameters)
    );
    dev->d3d_present_parameters.Windowed = TRUE;
    dev->d3d_present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    dev->d3d_present_parameters.BackBufferFormat
        = D3DFMT_UNKNOWN;  // Need to use an explicit format with alpha if
                           // needing per-pixel alpha composition.
    dev->d3d_present_parameters.EnableAutoDepthStencil = TRUE;
    dev->d3d_present_parameters.AutoDepthStencilFormat = D3DFMT_D16;
    dev->d3d_present_parameters.PresentationInterval
        = D3DPRESENT_INTERVAL_ONE;  // Present with vsync
    if (dev->d3d->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            window_handle,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &dev->d3d_present_parameters,
            &dev->d3d_device
        )
        < 0)
        return nullptr;

    return dev;
}

void ResetDevice(DeviceD3D* dev) {
    logging::debug("Resetting Direct3D device...");
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = dev->d3d_device->Reset(&dev->d3d_present_parameters);
    assert(hr != D3DERR_INVALIDCALL);
    ImGui_ImplDX9_CreateDeviceObjects();
}