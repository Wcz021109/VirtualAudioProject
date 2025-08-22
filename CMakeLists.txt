# 顶级CMakeList.txt文件

cmake_minimum_required(VERSION 3.16)
project(VirtualAudioManager
    VERSION 1.0.0
    DESCRIPTION "Virtual Audio Manager"
    LANGUAGES C CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 项目配置选项
option(BUILD_TESTS "Build Tests" ON)
option(BUILD_EXAMPLES "Build Examples" OFF)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含第三方库目录
include_directories(${CMAKE_SOURCE_DIR}/third_party)

# 添加子目录
add_subdirectory(src/audio_core)
add_subdirectory(src/device_management)
add_subdirectory(src/utils)

# 添加测试
if(BUIDL_TESTS)
    enable_testing()
    add_subdirectory(src/tests)
endif()

# 添加主应用程序
add_executable(VirtualAudioManager src/main.cpp)

# 链接依赖库
target_link_directories(VirtualAudioManager
    PRIVATE
        AudioCore
        DeviceManagement
        AudioUtils
)

# Windows特定设置
if(WIN32)
    # 添加Windows版本定义
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows10
    add_definitions(-DNTDDI_VERSION=0x0A000000) # Windows10

    add_definitions(-DUNICODE -D_UNICODE)

    # 链接Windows库
    target_link_libraries(VirtualAudioManager
        PRIVATE
            winmm
            setupapi
            ole32
    )
endif()