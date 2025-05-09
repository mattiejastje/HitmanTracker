#include "deviced3d.hpp"

#include "../logging.hpp"

void DeviceD3DDeleter::operator()(DeviceD3D *dev) const {
    logging::trace("Releasing Direct3D...");
    if (dev->g_pd3dDevice) {
        dev->g_pd3dDevice->Release();
    }
    if (dev->g_pD3D) {
        dev->g_pD3D->Release();
    }
}

std::unique_ptr<DeviceD3D, DeviceD3DDeleter> CreateDeviceD3D(HWND window_handle
) {
    logging::trace("Initializing Direct3D...");
    auto dev = std::unique_ptr<DeviceD3D, DeviceD3DDeleter>(new DeviceD3D);
    dev->g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!dev->g_pD3D) return nullptr;
    ZeroMemory(&dev->g_d3dpp, sizeof(dev->g_d3dpp));
    dev->g_d3dpp.Windowed = TRUE;
    dev->g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    dev->g_d3dpp.BackBufferFormat
        = D3DFMT_UNKNOWN;  // Need to use an explicit format with alpha if
                           // needing per-pixel alpha composition.
    dev->g_d3dpp.EnableAutoDepthStencil = TRUE;
    dev->g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    dev->g_d3dpp.PresentationInterval
        = D3DPRESENT_INTERVAL_ONE;  // Present with vsync
    if (dev->g_pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            window_handle,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &dev->g_d3dpp,
            &dev->g_pd3dDevice
        )
        < 0)
        return nullptr;

    return dev;
}
