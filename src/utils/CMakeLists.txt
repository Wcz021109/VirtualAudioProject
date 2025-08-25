# 工具库的CMake配置文件
# 工具库
add_library(AudioUtils STATIC
        include/Logger.h
        src/Logger.cpp
)

target_include_directories(AudioUtils PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 安装头文件
install(DIRECTORY include/ DESTINATION include/AudioUtils)