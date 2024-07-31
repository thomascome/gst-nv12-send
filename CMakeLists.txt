cmake_minimum_required(VERSION 3.10)

# Project name
project(SendNV12UDP)

# Find GStreamer and GStreamer App libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)

# Include directories
include_directories(${GSTREAMER_INCLUDE_DIRS})
include_directories(${GSTREAMER_APP_INCLUDE_DIRS})

# Add executable
add_executable(send_nv12_udp udp_nv12_send.cc)

# Link libraries
target_link_libraries(send_nv12_udp ${GSTREAMER_LIBRARIES} ${GSTREAMER_APP_LIBRARIES})

