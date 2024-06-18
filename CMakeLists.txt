cmake_minimum_required(VERSION 3.16.3)

project(ver)

set(CMAKE_CXX_STANDARD 17)

# Options (by default it does not build the viewer)
option(BUILD_VIEWER "Build the viewer" OFF)
set(PLATFORM "Desktop" CACHE STRING "Platform to build for (Desktop, Web)")

# Fetch raylib
if (BUILD_VIEWER)
  execute_process(
    COMMAND git clone https://github.com/raysan5/raylib.git --depth=1 --branch=5.0 ${CMAKE_CURRENT_SOURCE_DIR}/include/raylib
  )
endif()

# Set flags
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp") # Multi-threaded
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wcast-qual -Wshadow -Wpointer-arith") # Warnings

if (BUILD_VIEWER)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVIEWER")
endif()

if (PLATFORM STREQUAL "Desktop")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") # Multi-threaded
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
endif()

endif()

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) # Copy assets to build directory

message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Platform: ${PLATFORM}")

# Include directories
if (BUILD_VIEWER)
include_directories(SYSTEM include/raylib/src)
endif()
include_directories(SYSTEM include/)
include_directories(src/)

# Source files
file(GLOB SOURCES src/*.cc src/*.hh src/*.h src/**/*.cc src/**/*.hh src/**/*.h)
message(STATUS "Source files: ${SOURCES}")

if (NOT BUILD_VIEWER)
# Build main executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  m
)

else()
# Build viewer
add_subdirectory(include/raylib)

add_executable(viewer ${SOURCES})

target_link_libraries(viewer
  m
  raylib
)

endif()
