#include "hook.hpp"

HookPtr hitman_codename_47::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            Source{
                base_ptrs[2] + 0xFB16,
                // push esi
                // mov esi,[esp+18]
                {0x56, 0x8B, 0x74, 0x24, 0x18},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Label{100}},
            },
            Source{
                base_ptrs[1] + 0x173355,
                // inc ecx
                // xor esi,esi
                // test eax,eax
                {0x41, 0x33, 0xF6, 0x85, 0xC0},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{210}}, Label{200}},
            },
        },
        {
            // scene load
            Label{110},
            Code{0x56},                    // push esi
            Code{0x8B, 0x74, 0x24, 0x18},  // mov esi,[esp+18]
            Code{0x89, 0x35},              // mov [Label{150}],esi
            Ptr{Label{150}},               // ...
            Jump{Code{0xE9}, Label{100}},  // jmp Label{100}

            Align{4, 0xCC},
            Label{150},  // scene name
            Fill{0x40},

            Align{4, 0xCC},
            // increment mission time
            Label{210},
            Code{0x41},        // inc ecx
            Code{0x89, 0x0D},  // move [{Label{250}],ecx
            Ptr{Label{250}},   // ...
            Code{0x33, 0xF6},  // xor esi,esi
            Code{0x85, 0xC0},  // test eax,eax
            Jump{Code{0xE9}, Label{200}},

            Align{4, 0xCC},
            Label{250},  // mission time
            Fill{0x40},
        }
    );
}