cmake_minimum_required (VERSION 3.30)
project ("HitmanTracker")

find_package(CLI11 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable (HitmanTracker "HitmanTracker.cpp" "gui.cpp" "logger.cpp" "settings.cpp")
set_property(TARGET HitmanTracker PROPERTY CXX_STANDARD 20)
target_link_libraries(HitmanTracker PRIVATE CLI11::CLI11)
target_link_libraries(HitmanTracker PRIVATE imgui::imgui)
target_link_libraries(HitmanTracker PRIVATE spdlog::spdlog_header_only)
