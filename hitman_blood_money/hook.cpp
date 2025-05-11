#include "hook.hpp"

HookPtr hitman_blood_money::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        base_ptrs[0] + 0x0668C0,
        // original source code
        // push esi
        // mov esi,[esp+08]
        // mov eax,esi
        // push edi
        {0x56, 0x8B, 0x74, 0x24, 0x08, 0x8B, 0xC6, 0x57},
        // new source code (jumps to target code)
        {
            JumpSourceToTarget{0, 64},  // source+0 jmp target+64
            Nop{3}                      // source+5 nop
            // source+8 ...
        },
        // target code (stores edx and jumps to resume original source)
        {
            Zero{0x40},                    // target+0  (storage for scene name)
            Code{0x56},                    // target+40 push esi
            Code{0x57},                    // target+41 push edi
            Code{0x51},                    // target+42 push ecx
            Code{0x8B, 0x74, 0x24, 0x10},  // target+43 mov esi,[esp+10]
            Code{0xBF},                    // target+47 mov edi,target+0
            TargetPointer{0},              // ...
            Code{0xB9, 0x40, 0, 0, 0},     // target+4C mov ecx,40
            Code{0xFC},                    // target+51 cld
            // loopne jumps back here
            Code{0xAC, 0xAA},               // target+52 lodsb; stosb
            Code{0x84, 0xC0},               // target+54 test al, al
            Code{0xE0, 0xFA},               // target+56 loopne -6
            Code{0x8B, 0x74, 0x24, 0x10},   // target+58 mov esi,[esp+10]
            Code{0x8B, 0xC6},               // target+5C mov eax,esi
            Code{0x59},                     // target+5E pop ecx
            JumpTargetToSource{0x5F, 0x8},  // target+5F jmp source+8
        }
    );
}