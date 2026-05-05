#include "hook.hpp"

HookPtr hitman_codename_47::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            Source{
                base_ptrs[1] + 0x64CEB,
                // mov esi,ecx
                // mov ecx,[esi+0C]
                {0x8B, 0xF1, 0x8B, 0x4E, 0x0C},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Label{100}},
            },
        },
        {
            Label{110},
            Code{0x8B, 0x41, 0x08},        // mov eax,[ecx+08]
            Code{0x8B, 0x40, 0x14},        // mov eax,[eax+14]
            Code{0xA3},                    // mov [Label{150}],eax
            Ptr{Label{150}},               // ...
            Code{0x8B, 0xF1},              // mov esi,ecx
            Code{0x8B, 0x4E, 0x0C},        // mov ecx,[esi+0C]
            Jump{Code{0xE9}, Label{100}},  // jmp Label{100}

            Align{4, 0xCC},
            Label{150},     // difficulty
            Fill{4, 0xFF},  // initial value is -1 (unknown)
        }
    );
}