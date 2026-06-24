cmake_minimum_required (VERSION 3.30)
project ("HitmanTracker")

find_package(CLI11 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_subdirectory("mempeep")

add_executable (HitmanTracker
	"HitmanTracker.cpp"
	"gui.cpp"
	"logging.cpp"
	"settings.cpp"
	"game_info_registry.cpp"
	"game.cpp"
	"hook.cpp"
	"imgui_utils.cpp"
	"format_duration.cpp"
	"gui/deviced3d.cpp"
	"gui/ui.cpp"
	"gui/window.cpp"
	"spdlog.cpp"
	"mem/alloc.cpp"
	"mem/handle.cpp"
	"mem/read_write.cpp"
	"fnv1a.cpp"
	"hitman2_silent_assassin/game_info.cpp"
	"hitman2_silent_assassin/gui.cpp"
	"hitman2_silent_assassin/stats.cpp"
	"hitman_2016/game_info.cpp"
	"hitman_2016/gui.cpp"
	"hitman_absolution/game_info.cpp"
	"hitman_absolution/gui.cpp"
    "hitman_absolution/hook.cpp"
	"hitman_absolution/stats.cpp"
	"hitman_common/gui.cpp"
	"hitman_common/stats.cpp"
	"hitman_contracts/game_info.cpp"
	"hitman_contracts/gui.cpp"
	"hitman_contracts/stats.cpp"
	"hitman_codename_47/game_info.cpp"
	"hitman_codename_47/gui.cpp"
	"hitman_codename_47/stats.cpp"
	"hitman_blood_money/game_info.cpp"
	"hitman_blood_money/gui.cpp"
	"hitman_blood_money/hook.cpp"
	"hitman_blood_money/stats.cpp"
	"signal.cpp"
	"stats.cpp"
)
set_property(TARGET HitmanTracker PROPERTY CXX_STANDARD 20)
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
target_link_libraries(HitmanTracker PRIVATE CLI11::CLI11)
target_link_libraries(HitmanTracker PRIVATE imgui::imgui)
target_link_libraries(HitmanTracker PRIVATE spdlog::spdlog_header_only)
target_link_libraries(HitmanTracker PRIVATE mempeep)
