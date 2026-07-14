#include "deviced3d.hpp"

#include <imgui_impl_dx9.h>
#include <spdlog/spdlog.h>

void imgui_app::DeviceD3DDeleter::operator()(DeviceD3D* dev) const {
    spdlog::debug("Releasing Direct3D...");
    if (dev) {
        if (dev->d3d_device) {
            dev->d3d_device->Release();
        }
        if (dev->d3d) {
            dev->d3d->Release();
        }
        delete dev;
    }
}

imgui_app::DeviceD3DPtr imgui_app::CreateDeviceD3D(HWND window_handle) {
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

bool imgui_app::ResetDevice(DeviceD3D& dev) {
    spdlog::debug("Resetting Direct3D device...");
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = dev.d3d_device->Reset(&dev.state.present_parameters);
    if (hr != D3D_OK) {
        spdlog::warn("Direct3D device reset failed: {:#x}", hr);
        return false;
    }
    ImGui_ImplDX9_CreateDeviceObjects();
    return true;
}

HRESULT imgui_app::RenderAndPresent(DeviceD3D& dev) {
    spdlog::trace("Rendering...");
    dev.d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
    dev.d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    dev.d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    if (dev.d3d_device->BeginScene() >= 0) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        dev.d3d_device->EndScene();
    }
    return dev.d3d_device->Present(nullptr, nullptr, nullptr, nullptr);
}

bool imgui_app::IsDeviceReady(DeviceD3D& dev) {
    HRESULT hr = dev.d3d_device->TestCooperativeLevel();
    if (hr == D3D_OK) return true;
    if (hr == D3DERR_DEVICELOST) {
        spdlog::debug("Device lost");
        ::Sleep(10);
        return false;
    }
    if (hr == D3DERR_DEVICENOTRESET) {
        if (!imgui_app::ResetDevice(dev)) return false;
    }
    spdlog::debug("Device recovered");
    return true;
}
