cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME BASIC)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    sys/system.c
    sys/sys.c
    sys/delay.c
    hardware/OLED.c
    hardware/OLED_Data.c
    NVIC/timer.c
    NVIC/UART_NVIC.c
    NVIC/uart_execute.c
    hardware/APP_MAIN.c
    hardware/task_execute.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    hardware
    NVIC
    sys
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# HAL/StdPeriph drivers sometimes intentionally keep unused parameters for API compatibility.
# Other IDEs often hide these warnings; keep output clean for GCC builds here.
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wno-unused-parameter
)

# Keep OLED font tables unchanged while silencing style-only warnings
# for dense const initializers in that generated data file.
set_source_files_properties(hardware/OLED_Data.c PROPERTIES
    COMPILE_OPTIONS "-Wno-missing-braces;-Wno-missing-field-initializers"
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
