#include "hook.hpp"

#include "../mem/read_write.hpp"

bool hitman_absolution::hook_ready(void* handle, const BasePtrs& base_ptrs) {
    // initial play/options/register/quit dialog hangs if we try hook too early
    // so only declare ready when the game's internal timer has started
    const auto& base_ptr = base_ptrs.at(0);
    auto game_time
        = read<int64_t>(handle, base_ptr + 0xE24730 + 0x18).value_or(0);
    return game_time != 0;
}

HookPtr hitman_absolution::hook(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    const auto& base_ptr = base_ptrs.at(0);
    const auto game_stats_ptr = base_ptr + 0x5B2538;
    return install_hook(
        handle,
        {
            Source{
                base_ptr + 0x559B87,
                // mov [ebp-28],00000002
                {0xC7, 0x45, 0xD8, 2, 0, 0, 0},
                // new source code (jumps to target code)
                {Jump{Code{0xE9}, Label{110}}, Fill{2, 0x90}, Label{100}},
            },
        },
        {
            Label{110},
            Code{0xC7, 0x05},              // mov [Label{150}],1
            Ptr{Label{150}},               // ...
            Code{1, 0, 0, 0},              // ...
            Code{0xC7, 0x45, 0xD8},        // mov [ebp-28],00000002
            Code{2, 0, 0, 0},              // ...
            Jump{Code{0xE9}, Label{100}},  // jmp Label{100}

            Align{4, 0xCC},
            Label{150},  // spotted
            Fill{0x40},
        }
    );
}