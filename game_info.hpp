#pragma once

#include <any>
#include <filesystem>
#include <functional>
#include <span>

#include "base_ptrs.hpp"
#include "hook.hpp"
#include "imgui_app/ui.hpp"
#include "label_ptrs.hpp"
#include "pe.hpp"
#include "stats.hpp"

using GameGui
    = std::function<void(std::span<ImFont *>, const std::any & /* stats */)>;

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
    std::string tag;
    GameMethods methods;
    std::function<std::any()> make_remote_state;
    std::function<std::any()> make_stats;
    // first module name is always exe name
    std::vector<ModuleInfo> module_infos;
};

template <class Spec>
using MakeMethods = std::function<GameMethods(const Spec &)>;

template <class Spec>
using MakeModuleInfos = std::function<std::vector<ModuleInfo>(const Spec &)>;

template <class Spec>
void register_game_variants(
    std::vector<GameInfo> &registry,
    std::string_view tag_prefix,
    std::span<const Spec> specs,
    MakeMethods<Spec> make_methods,
    MakeModuleInfos<Spec> make_module_infos,
    std::function<std::any()> make_remote_state,
    std::function<std::any()> make_stats
) {
    for (const auto &spec : specs) {
        registry.emplace_back(
            GameInfo{
                .tag = std::string(tag_prefix) + spec.tag_suffix,
                .methods = make_methods(spec),
                .make_remote_state = make_remote_state,
                .make_stats = make_stats,
                .module_infos = make_module_infos(spec),
            }
        );
    }
}

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
