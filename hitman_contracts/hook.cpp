#include "hook.hpp"

HookPtr hitman_contracts::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            // set property (note: same as H2SA, except for the base pointer)
            Source{
                base_ptrs[0] + 0x212540,
                // mov edx,[esp+04]
                // push 00
                {0x8B, 0x54, 0x24, 0x04, 0x6A, 0x00},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{210}}, Code{0x90}, Label{200}},
            },
        },
        {
            // set property (note: same as H2SA, except for "lethed" pointer)
            Label{210},
            Code{0x8B, 0x54, 0x24, 0x04},  // mov edx,[esp+04]
            Code{0x6A, 0x00},              // push 00
            Code{0x81, 0xFA},  // cmp edx, hitmancontracts.exe+363F38
            Ptr{base_ptrs[0] + 0x363F38},        // ...
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