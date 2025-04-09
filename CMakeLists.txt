cmake_minimum_required (VERSION 3.30)
project ("HitmanTracker")

# Add source to this project's executable.
add_executable (HitmanTracker "HitmanTracker.cpp")
set_property(TARGET HitmanTracker PROPERTY CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
