#include "deviced3d.hpp"

#include <imgui_impl_dx9.h>
#include <spdlog/spdlog.h>

void imgui_app::DeviceD3DDeleter::operator()(DeviceD3D* dev) const {
    spdlog::debug("Releasing Direct3D...");
    if (dev->d3d_device) {
        dev->d3d_device->Release();
    }
    if (dev->d3d) {
        dev->d3d->Release();
    }
}

std::unique_ptr<imgui_app::DeviceD3D, imgui_app::DeviceD3DDeleter>
imgui_app::CreateDeviceD3D(HWND window_handle) {
    spdlog::debug("Initializing Direct3D...");
    auto dev = std::unique_ptr<DeviceD3D, DeviceD3DDeleter>(new DeviceD3D);
    dev->d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!dev->d3d) return nullptr;
    ZeroMemory(
        &dev->state.present_parameters, sizeof(dev->state.present_parameters)
    );
    dev->state.present_parameters.Windowed = TRUE;
    dev->state.present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    dev->state.present_parameters.BackBufferFormat
        = D3DFMT_UNKNOWN;  // Need to use an explicit format with alpha if
                           // needing per-pixel alpha composition.
    dev->state.present_parameters.EnableAutoDepthStencil = TRUE;
    dev->state.present_parameters.AutoDepthStencilFormat = D3DFMT_D16;
    dev->state.present_parameters.PresentationInterval
        = D3DPRESENT_INTERVAL_ONE;  // Present with vsync
    if (dev->d3d->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            window_handle,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &dev->state.present_parameters,
            &dev->d3d_device
        )
        < 0)
        return nullptr;

    return dev;
}

void imgui_app::ResetDevice(DeviceD3D* dev) {
    spdlog::debug("Resetting Direct3D device...");
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = dev->d3d_device->Reset(&dev->state.present_parameters);
    assert(hr != D3DERR_INVALIDCALL);
    ImGui_ImplDX9_CreateDeviceObjects();
    dev->state.is_lost = false;
}

HRESULT imgui_app::RenderAndPresent(DeviceD3D* dev) {
    spdlog::trace("Rendering...");
    dev->d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
    dev->d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    dev->d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    if (dev->d3d_device->BeginScene() >= 0) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        dev->d3d_device->EndScene();
    }
    HRESULT result
        = dev->d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
    if (result == D3DERR_DEVICELOST) dev->state.is_lost = true;
    return result;
}
