# 测试单元的CMake配置文件
# 查找Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7186c34b8af7a41d87a6.zip
)

FetchContent_MakeAvailable(googletest)

# 添加测试可执行文件
add_executable(VirtualAudioTests
        unit_tests/TestDeviceEnumeration.cpp
)

target_link_libraries(VirtualAudioTests
        PRIVATE
        DeviceManagement
        gtest_main
)

# 添加测试
gtest_discover_tests(VirtualAudioTests)