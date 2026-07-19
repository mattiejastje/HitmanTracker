cmake_minimum_required (VERSION 3.30)
project ("HitmanTracker")

find_package(CLI11 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# spdlog backend needs mempeep to use fmtlib
set(MEMPEEP_USE_FMTLIB ON CACHE BOOL "" FORCE)
add_subdirectory("mempeep")

add_executable (HitmanTracker
	"HitmanTracker.cpp"
	"color.cpp"
	"gui.cpp"
	"settings.cpp"	
	"game.cpp"
	"hook.cpp"
	"format_duration.cpp"
	"imgui_app/app_window.cpp"
	"imgui_app/deviced3d.cpp"
	"imgui_app/fonts.cpp"
	"imgui_app/key_value_table.cpp"
	"imgui_app/text.cpp"
	"imgui_app/ui.cpp"
	"imgui_app/window.cpp"
	"imgui_app/window_class.cpp"
	"spdlog.cpp"
	"mem/alloc.cpp"
	"mem/handle.cpp"
	"mem/read_write.cpp"
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
	"hitman_common/read_lethed.cpp"
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
	"pe.cpp"
	"settings_gui.cpp"
	"shell.cpp"
	"signal.cpp"
	"stats.cpp"
	"timer.cpp"
)
set_property(TARGET HitmanTracker PROPERTY CXX_STANDARD 23)
target_link_options(HitmanTracker PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
    $<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup>
)
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
set(APP_VERSION "0.0.0-dev" CACHE STRING "Application version")
target_compile_definitions(HitmanTracker PRIVATE APP_VERSION="${APP_VERSION}")
target_link_libraries(HitmanTracker PRIVATE CLI11::CLI11)
target_link_libraries(HitmanTracker PRIVATE imgui::imgui)
target_link_libraries(HitmanTracker PRIVATE spdlog::spdlog)
target_link_libraries(HitmanTracker PRIVATE mempeep)
