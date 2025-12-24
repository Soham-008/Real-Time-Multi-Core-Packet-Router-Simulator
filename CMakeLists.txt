cmake_minimum_required(VERSION 3.26)
project(packet_router_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threading library
find_package(Threads REQUIRED)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Optimization flags for Release
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Enable profiling (optional - comment out for normal builds)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

add_executable(packet_router_simulator
        main.cpp
        router.cpp
        statistics.h
        statistics.h
        thread_safe_queue.h
        routing_core.h
        router.h
        router.cpp
        event.h
)

target_link_libraries(packet_router_simulator
        Threads::Threads
)