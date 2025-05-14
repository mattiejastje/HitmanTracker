#include "hook.hpp"

HookPtr hitman_blood_money::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        {
            Source{
                base_ptrs[0] + 0x0668C0,
                // original source code
                // push esi
                // mov esi,[esp+08]
                // mov eax,esi
                // push edi
                {0x56, 0x8B, 0x74, 0x24, 0x08, 0x8B, 0xC6, 0x57},
                // new source code (jumps to target code)
                {Jump{Label{200}}, Fill{3, 0x90}, Label{100}},
            },
        },
        // target code:
        // copies the scene name
        // converts backslashes to forward slashes
        // converts upper case to lower case
        {
            // new code
            Label{200},
            Code{0x56},                    // push esi
            Code{0x57},                    // push edi
            Code{0x51},                    // push ecx
            Code{0x8B, 0x74, 0x24, 0x10},  // mov esi,[esp+10]
            Code{0xBF},                    // mov edi,Label{250}
            Pointer{Label{250}},           // ...
            Code{0xB9, 0x40, 0, 0, 0},     // mov ecx,40
            Code{0xFC},                    // cld

            // jne -27 jumps back here
            Code{0xAC},                    // lodsb
            Code{0x3C, 0x5C},              // cmp al,'\'
            Code{0x75, 0x04},              // jne short 4
            Code{0xB0, 0x2F},              // mov al,'/'
            Code{0xEB, 0x0A},              // jmp A
            Code{0x3C, 0x41},              // cmp al,'A'
            Code{0x7C, 0x06},              // jl short 6
            Code{0x3C, 0x5A},              // cmp al,'Z'
            Code{0x7F, 0x02},              // jg short 2
            Code{0x0C, 0x20},              // or al,20
            Code{0xAA},                    // stosb
            Code{0x84, 0xC0},              // test al, al
            Code{0x74, 0x03},              // jz 3
            Code{0x49},                    // dec ecx
            Code{0x75, 0xE5},              // jnz -27
            Code{0x8B, 0x74, 0x24, 0x10},  // mov esi,[esp+10]
            Code{0x8B, 0xC6},              // mov eax,esi
            Code{0x59},                    // pop ecx
            Jump{Label{100}},              // jmp Label{100}

            // storage for scene name
            Align{4, 0xCC},
            Label{250},
            Fill{0x40},
        }
    );
}