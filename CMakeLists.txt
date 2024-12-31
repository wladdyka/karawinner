cmake_minimum_required(VERSION 3.14)
project(karawinner)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Define the URL and file name for the prebuilt release
set(INTERCEPTION_RELEASE_URL "https://github.com/wladdyka/interception/releases/download/v1.0.1/interception.zip")
set(INTERCEPTION_ZIP "${CMAKE_BINARY_DIR}/interception.zip")
set(INTERCEPTION_DIR "${CMAKE_BINARY_DIR}/interception/library/x64")

# Add custom command to download the precompiled release
if(NOT EXISTS ${INTERCEPTION_ZIP})
    message(STATUS "Downloading Interception prebuilt release...")
    file(DOWNLOAD ${INTERCEPTION_RELEASE_URL} ${INTERCEPTION_ZIP} SHOW_PROGRESS)
endif()

# Add custom command to extract the release
if(NOT EXISTS ${INTERCEPTION_DIR})
    message(STATUS "Extracting Interception release...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INTERCEPTION_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ${INTERCEPTION_ZIP}
            WORKING_DIRECTORY ${INTERCEPTION_DIR})
endif()

# Add executable for the main application
add_executable(karawinner main.cpp)

message(STATUS "Interception directory: ${INTERCEPTION_DIR}")

# Link the Interception library
target_include_directories(karawinner PRIVATE ${INTERCEPTION_DIR}/interception/library)
target_link_libraries(karawinner PRIVATE ${INTERCEPTION_DIR}/interception/library/x64/interception.lib)
