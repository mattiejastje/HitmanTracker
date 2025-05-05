cmake_minimum_required (VERSION 3.30)
project ("HitmanTracker")

find_package(CLI11 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable (HitmanTracker
	"HitmanTracker.cpp"
	"gui.cpp"
	"logger.cpp"
	"settings.cpp"
	"game.cpp"
	"read_process_memory.cpp"
	"handle.cpp"
	"format_duration.cpp"
	"hitman2_silent_assassin/gui.cpp"
	"hitman2_silent_assassin/stats.cpp"
	"hitman_contracts/gui.cpp"
	"hitman_codename_47/gui.cpp"
	"hitman_blood_money/gui.cpp"
	"hitman_blood_money/stats.cpp"
)
set_property(TARGET HitmanTracker PROPERTY CXX_STANDARD 20)
add_definitions(-DWIN32_LEAN_AND_MEAN)
target_link_libraries(HitmanTracker PRIVATE CLI11::CLI11)
target_link_libraries(HitmanTracker PRIVATE imgui::imgui)
target_link_libraries(HitmanTracker PRIVATE spdlog::spdlog_header_only)
