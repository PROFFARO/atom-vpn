cmake_minimum_required(VERSION 3.10)
project(AtomVPN VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/WinDivert/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/wintun/api)

# Gather all source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.cc"
    "src/*.cxx"
)

add_executable(AtomVPN ${SOURCES})

# Link with the WinDivert library
target_link_libraries(AtomVPN ${PROJECT_SOURCE_DIR}/lib/WinDivert/x64/WinDivert.lib)