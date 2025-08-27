# 测试单元的CMake配置文件
# 查找Google Test
# 使用离线包
include(FetchContent)

FetchContent_Declare(
        googletest
        URL ${CMAKE_SOURCE_DIR}/third_party/googletest-1.17.0.zip
        URL_HASH SHA256=40d4ec942217dcc84a9ebe2a68584ada7d4a33a8ee958755763278ea1c5e18ff
)

FetchContent_MakeAvailable(googletest)

# 添加测试可执行文件
add_executable(VirtualAudioTests
        unit_tests/TestDeviceEnumeration.cpp
)

target_link_libraries(VirtualAudioTests
        PRIVATE
        DeviceManagement
        AudioUtils
        gtest
        gtest_main
)

target_include_directories(VirtualAudioTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${googletest_SOURCE_DIR}/googletest/include
)

# 添加测试目标
add_custom_target(run_tests
        COMMAND VirtualAudioTests
        DEPENDS VirtualAudioTests
        COMMENT "Running tests..."
)