cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(remote_cli VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_path(LIBMODBUS_INCLUDE_DIR modbus/modbus.h)
find_library(LIBMODBUS_LIBRARY NAMES modbus)

if (LIBMODBUS_INCLUDE_DIR AND LIBMODBUS_LIBRARY)
    message(STATUS "Found libmodbus")
    include_directories(${LIBMODBUS_INCLUDE_DIR})

else()
    message(FATAL_ERROR "libmodbus not found")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

if (LIBMODBUS_VERSION VERSION_LESS "3.1.2")
    set(USE_OLD_MODBUS_API TRUE)
    add_definitions(-DLIBMODBUS_PRE_312)
    message(STATUS "Using old modbus API behavior")
else()
    set(USE_OLD_MODBUS_API FALSE)
    message(STATUS "Using new modbus API behavior")
endif()

# Add the executable target
add_executable(remote_cli
    main.cpp
    cli/src/Prompt.cpp
    cli_commands.cpp
    modbus_registers.cpp
)

add_custom_target(pre_build_command
    COMMAND git -C ${CMAKE_SOURCE_DIR}/cli checkout main
    COMMENT "Checking out submodule to 'my-branch' before build"
)

# Make sure your app depends on the pre-build step
add_dependencies(remote_cli pre_build_command)


# Include directories for header files
target_include_directories(remote_cli PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_include_directories(${PROJECT_NAME} PRIVATE
    "cli/src/"
)

target_link_libraries(${PROJECT_NAME} modbus)
target_compile_options(remote_cli PRIVATE
    -Wall
    -Werror
)
