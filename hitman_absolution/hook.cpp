#include "hook.hpp"

#include "../mem/read_write.hpp"

GameHookReady hitman_absolution::hook_ready(Version version) {
    const uint32_t time_manager_offset
        = version == Version::Steam ? 0xE24730 : 0xC88580;
    return [time_manager_offset](void* handle, const BasePtrs& base_ptrs) {
        // initial play/options/register/quit dialog hangs if we try hook too
        // early so only declare ready when the game's internal timer has
        // started
        const auto& base_ptr = base_ptrs.at(0);
        auto game_time
            = read<int64_t>(handle, base_ptr + time_manager_offset + 0x18)
                  .value_or(0);
        return game_time != 0;
    };
}

GameHook hitman_absolution::hook(Version version) {
    return [version](std::shared_ptr<void> handle, const BasePtrs& base_ptrs) {
        const auto& base_ptr = base_ptrs.at(0);
        const int32_t offset
            = (version == Version::Steam ? 0x559B87 : 0x51E8C7);
        // mov [ebp-28],00000002 (steam)
        // mov [ebp-20],00000002 (gog)
        const uint8_t ebp_arg = (version == Version::Steam ? 0xD8 : 0xE0);
        const Code source_code = Code{0xC7, 0x45, ebp_arg, 2, 0, 0, 0};
        return install_hook(
            handle,
            {
                Source{
                    base_ptr + offset,
                    source_code,
                    // new source code (jumps to target code)
                    {Jump{Code{0xE9}, Label{110}}, Fill{2, 0x90}, Label{100}},
                },
            },
            {
                Label{110},
                Code{0xC7, 0x05},              // mov [Label{150}],1
                Ptr{Label{150}},               // ...
                Code{1, 0, 0, 0},              // ...
                source_code,                   // mov [ebp-28],00000002
                Jump{Code{0xE9}, Label{100}},  // jmp Label{100}

                Align{4, 0xCC},
                Label{150},  // spotted
                Fill{0x40},
            }
        );
    };
}
