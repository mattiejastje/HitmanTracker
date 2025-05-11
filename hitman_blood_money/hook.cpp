#include "hook.hpp"

HookPtr hitman_blood_money::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return install_hook(
        handle,
        base_ptrs[0] + 0x04BAD8,
        // original source code
        {0x89, 0x56, 0x38, 0xC7, 0x04, 0x81, 0, 0, 0, 0},
        // new source code (jumps to target code)
        {
            JumpSourceToTarget{0, 4},  // source+0 jmp target+4
            Nop{5}                     // source+5 nop
            // source+10 ...
        },
        // target code (stores edx and jumps to resume original source)
        {
            Zero{4},           // target+0  dd 0 (storage for shots fired)
            Code{0x89, 0x15},  // target+4  mov dword ptr ds:[target+0],edx
            TargetPointer{0},  // target+6
            Code{0x89, 0x56, 0x38},      // target+10 mov [esi+38],edx
            Code{0xC7, 0x04, 0x81},      // target+13 mov [ecx+eax*4],00000000
            Zero{4},                     // target+16
            JumpTargetToSource{20, 10},  // target+20 jmp source+10
        }
    );
}