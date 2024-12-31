cmake_minimum_required(VERSION 3.14)
project(karawinner)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Fetch Interception library from GitHub
include(FetchContent)
FetchContent_Declare(
        Interception
        GIT_REPOSITORY https://github.com/wladdyka/interception
        GIT_TAG master
)
FetchContent_MakeAvailable(Interception)

# Add executable
add_executable(karawinner main.cpp)

# Link Interception library
# Include the correct path for the Interception headers
target_include_directories(karawinner PRIVATE ${interception_SOURCE_DIR}/library)
target_link_libraries(karawinner interception)
