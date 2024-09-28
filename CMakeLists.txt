cmake_minimum_required(VERSION 3.29)
project(Trainering)

set(CMAKE_CXX_STANDARD 20)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src/")

add_executable(Trainering)

target_sources(Trainering PRIVATE ${PROJECT_SOURCES})

target_include_directories(Trainering PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(Trainering PRIVATE raylib)

target_compile_features(Trainering PRIVATE cxx_std_17)  # Enable C++17 features