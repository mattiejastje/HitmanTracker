#include "hook.hpp"

HookPtr hitman2_silent_assassin::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            Source{
                base_ptrs[0] + 0x11C3F0,
                // inc [ecx+000011C7]
                {0xFF, 0x81, 0xC7, 0x11, 0x00, 0x00},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Code{0x90}, Label{100}},
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
        }
    );
}