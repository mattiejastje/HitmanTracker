#include "hook.hpp"

HookPtr hitman2_silent_assassin::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            // increment shots fired
            Source{
                base_ptrs[0] + 0x11C3F0,
                // inc [ecx+000011C7]
                {0xFF, 0x81, 0xC7, 0x11, 0x00, 0x00},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Code{0x90}, Label{100}},
            },
            // set property
            Source{
                base_ptrs[0] + 0x1B3050,
                // mov edx,[esp+04]
                // push 00
                {0x8B, 0x54, 0x24, 0x04, 0x6A, 0x00},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{210}}, Code{0x90}, Label{200}},
            },
        },
        {
            // increment shots fired
            Label{110},
            Code{0x89, 0x0D},                          // mov [Label{150}],ecx
            Ptr{Label{150}},                           // ...
            Code{0xFF, 0x81, 0xC7, 0x11, 0x00, 0x00},  // inc [ecx+000011C7]
            Jump{Code{0xE9}, Label{100}},              // jmp Label{100}

            Align{4, 0xCC},
            Label{150},  // shots fired base pointer
            Fill{0x40},

            // set property
            Label{210},
            Code{0x8B, 0x54, 0x24, 0x04},        // mov edx,[esp+04]
            Code{0x6A, 0x00},                    // push 00
            Code{0x81, 0xFA},                    // cmp edx, hitman2.exe+28AA18
            Ptr{base_ptrs[0] + 0x28AA18},        // ...
            Jump{Code{0x0F, 0x85}, Label{200}},  // jne Label{200}
            Code{0x8B, 0x44, 0x24, 0x0C},        // mov eax,[esp+0C]
            Code{0xA3},                          // mov [Label{250}],eax
            Ptr{Label{250}},                     // ...
            Jump{Code{0xE9}, Label{200}},        // jmp Label{200}

            Align{4, 0xCC},
            Label{250},  // difficulty value
            Fill{0x40},
        }
    );
}