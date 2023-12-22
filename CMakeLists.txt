cmake_minimum_required(VERSION 3.16.3)

project(ver)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp") # Multi-threaded
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") # Multi-threaded
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wcast-qual -Wshadow -Wpointer-arith") # Warnings
endif()

include_directories(SYSTEM include/ )
include_directories(src/)

file(GLOB SOURCES src/*.cc src/*.hh src/*.h src/**/*.cc src/**/*.hh src/**/*.h)
message(STATUS "Source files: ${SOURCES}")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  m
)
