# 设备管理单元CMake配置文件
# 设备管理库
add_library(DeviceManagement STATIC
    include/AudioDeviceManager.h
    src/AudioDeviceManager.cpp
)

target_include_directories(DeviceManagement PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(DeviceManagement PUBLIC
    ole32
)

if(WIN32)
    target_link_libraries(DeviceManagement PUBLIC
        setupapi
    )
endif()

# 安装头文件
install(DIRECTORY include/ DESTINATION include/DeviceManagement)