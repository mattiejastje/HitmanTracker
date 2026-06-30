#pragma once

#include <any>
#include <filesystem>
#include <functional>

#include "base_ptrs.hpp"
#include "gui/ui.hpp"
#include "hook.hpp"
#include "label_ptrs.hpp"
#include "pe.hpp"
#include "stats.hpp"

using GameGui
    = std::function<void(const Fonts &, const std::any & /* stats */)>;

using GameHook
    = std::function<HookPtr(std::shared_ptr<void>, const BasePtrs &)>;

using GameHookReady = std::function<bool(void *, const BasePtrs &)>;

using GameStatsSlow = std::function<bool(
    const std::filesystem::path &,
    void *,
    const BasePtrs &,
    const LabelPtrs &,
    std::any &, /* remote_state */
    std::any &  /* stats */
)>;

using GameStatsFast = std::function<
    bool(void *, const BasePtrs &, const LabelPtrs &, std::any & /* stats */)>;

struct GameMethods {
    GameGui gui;                // called every frame for displaying stats
    GameHook hook;              // called once when we are ready to hook
    GameHookReady hook_ready;   // called to check if we are ready to hook
    GameStatsSlow update_slow;  // called every 0.1 seconds
    GameStatsFast update_fast;  // called every frame
};

struct ModuleInfo {
    std::string name;
    PeId pe_id;
};

struct GameInfo {
    std::string name;
    GameMethods methods;
    std::function<std::any()> make_remote_state;
    std::function<std::any()> make_stats;
    // first module name is always exe name
    std::vector<ModuleInfo> module_infos;
};

inline bool stats_nothing_slow(
    const std::filesystem::path &exe_path,
    void *handle,
    const BasePtrs &base_ptrs,
    const LabelPtrs &label_ptrs,
    std::any &remote_state_any,
    std::any &stats_any
) {
    return true;
}

inline bool stats_nothing_fast(
    void *handle,
    const BasePtrs &base_ptrs,
    const LabelPtrs &label_ptrs,
    std::any &stats_any
) {
    return true;
}

inline HookPtr hook_nothing(
    std::shared_ptr<void> handle, const BasePtrs &base_ptrs
) {
    // non-null hook pointer means success so return stub
    return HookPtr{new Hook{}};
}

inline bool hook_immediately_ready(void *handle, const BasePtrs &base_ptrs) {
    return true;
}
