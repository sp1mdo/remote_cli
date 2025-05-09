cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(cli VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable target
add_executable(cli
    main.cpp
    cli/src/Prompt.cpp
    cli_commands.cpp
    modbus_registers.cpp
)

# Include directories for header files
target_include_directories(cli PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_include_directories(${PROJECT_NAME} PRIVATE
    "inc"
    "../../"
)

target_link_libraries(${PROJECT_NAME} modbus)

target_compile_options(cli PRIVATE
    -Wall
    -Werror
)
