//  设备管理头文件
#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <comdef.h>

//  设备类型枚举
enum class DeviceType {
    Physical,
    Virtual,
    Unknown
};//DeviceType

//  设备方向枚举
enum class DeviceDirection {
    Input,
    Output
};//DeviceType

//  音频设备信息结构
struct AudioDevice {
    std::wstring id;
    std::wstring name;
    std::wstring enumerator;
    DeviceType type;
    DeviceDirection direction;
    bool isDefault = false;
};//AudioDevice

//  设备客户端前向声明
class CMMNotificationClient;

//  音频设备管理器类
class AudioDeviceManager {
public:
    AudioDeviceManager();
    ~AudioDeviceManager();

    //  初始化COM和设备枚举器
    bool Initialize();

    //  枚举指定类型的设备
    bool PopulateDeviceLists();

    //  获取设备列表
    const std::vector<AudioDevice>& GetPhysicalInputs() const { return physicalInputs; }
    const std::vector<AudioDevice>& GetVirtualInputs() const { return virtualInputs; }
    const std::vector<AudioDevice>& GetPhysicalOutputs() const { return physicalOutputs; }
    const std::vector<AudioDevice>& GetVirtualOutputs() const { return virtualOutputs; }

    //  注册设备变化通知
    bool RegisterDeviceNotifications();
    bool UnregisterDeviceNotifications();

private:
    //  枚举指定类型的设备
    HRESULT EnumerateDevices(EDataFlow dataFlow, std::vector<AudioDevice>& devices);

    //  设备分类逻辑
    DeviceType ClassifyDevice(IMMDevice* pDevice ,IPropertyStore* pProps);

    //  检查是否为默认设备
    bool CheckIfDefaultDevice(IMMDevice* pDevice, EDataFlow dataFlow);

    //  COM和MMDevice接口
    IMMDeviceEnumerator* pEnumerator;

    //  设备列表
    std::vector<AudioDevice> physicalInputs;
    std::vector<AudioDevice> virtualInputs;
    std::vector<AudioDevice> physicalOutputs;
    std::vector<AudioDevice> virtualOutputs;

    //  设备通知客户端
    CMMNotificationClient* pNotificationClient;

    //  初始化状态
    bool isInitialized = false;
};//AudioDeviceManager