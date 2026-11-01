cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# 嵌入式裸机项目, 告诉CMake不要用宿主机(Windows)的链接选项
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME wsy)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# ---- MCU编译和链接选项 (从CubeMX工具链文件提取) ----
set(TARGET_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${TARGET_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${TARGET_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS_DEBUG          "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE        "-Os -g0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TARGET_FLAGS}")
set(TOOLCHAIN_LINK_LIBRARIES "m")

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
    Core/Inc/zj_rtc.h)

# 链接脚本和链接选项 (target级别, 避免全局FLAGS被重复追加)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T "${CMAKE_SOURCE_DIR}/STM32F103xx_FLASH.ld"
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    -Wl,--print-memory-usage
    -u _printf_float
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
