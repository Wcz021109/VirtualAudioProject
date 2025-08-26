//  主程序
#include "AudioDeviceManager.h"
#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <vector>

#include "device_management/include/AudioDeviceManager.h"
#include "utils/include/Logger.h"

void PrintDeviceList(const std::vector<AudioDevice> &devices, const std::wstring &title) {
	std::wcout << L"\n===" << title << L"(" << devices.size() << L")===" << std::endl;
	for (size_t i = 0; i < devices.size(); i++) {
		const auto &device = devices[i];
		std::wcout << L"[" << i + 1 << L"]" << (device.isDefault ? L"[Default]" : L"") << device.name << std::endl;
		std::wcout << L"    ID: " << device.id << std::endl;
		std::wcout << L"    Type: " << (device.type == DeviceType::Physical ? L"Physical" : L"Virtual") << std::endl;
		std::wcout << L"    Enumerator: " << device.enumerator << std::endl;
	}
}

int main() {
	Logger::GetInstance().SetLogLevel(LogLevel::INFO);
	Logger::GetInstance().Info("Starting Virtual Audio Manager...");

	AudioDeviceManager manager;

	if (!manager.Initialize()) {
		Logger::GetInstance().Error("Failed to initialize AudioDeviceManager");
		return 1;
	}

	if (!manager.PopulateDeviceLists()) {
		Logger::GetInstance().Error("Failed to initialize populate device lists");
	}

	//  注册设备变化通知
	if (!manager.RegisterDeviceNotifications()) {
		Logger::GetInstance().Error("Failed to register device notifications");
	}

	//  打印设备列表
	PrintDeviceList(manager.GetPhysicalInputs(),L"Physical Inputs");
	PrintDeviceList(manager.GetVirtualInputs(),L"Virtual Inputs");
	PrintDeviceList(manager.GetPhysicalOutputs(),L"Physical Outputs");
	PrintDeviceList(manager.GetVirtualOutputs(),L"Virtual Outputs");

	Logger::GetInstance().Info("Device enumeration completed. Press ENTER to exit...");
	std::cin.get();

	return 0;
}
