//  设备管理实现文件
#include "AudioDeviceManager.h"
#include <iostream>
#include <comdef.h>

#pragma comment(lib, "Ole32.lib")

//  设备通知客户端
class CMMNotificationClient : public IMMNotificationClient {
public:
	CMMNotificationClient(AudioDeviceManager *pManager) : pManager(pManager), refCount(1) {}

	//  IUnknown方法
	ULONG STDMETHODCALLTYPE AddRef() override {
		return InterlockedIncrement(&refCount);
	} //IUnknown.AddRef

	ULONG STDMETHODCALLTYPE Release() override {
		ULONG ulRef = InterlockedDecrement(&refCount);
		if (ulRef == 0) {
			delete this;
		}
		return ulRef;
	} //IUnknown.Release

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvInterface) override {
		if (riid == IID_IUnknown) {
			AddRef();
			*ppvInterface = (IUnknown *) this;
		} else if (riid == __uuidof(IMMNotificationClient)) {
			AddRef();
			*ppvInterface = (IMMNotificationClient *) this;
		} else {
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	//  IMMNotificationClient方法
	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override {
	std:
		std::wcout << L"Device state changed: " << pwstrDeviceId << std::endl;
		if (pManager) {
			pManager->PopulateDeviceLists();
		}
		return S_OK;
	} //DeviceStateChange

	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
	std:
		std::wcout << L"Device Added: " << pwstrDeviceId << std::endl;
		if (pManager) {
			pManager->PopulateDeviceLists();
		}
		return S_OK;
	} //DeviceAdded

	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
	std:
		std::wcout << L"Device Removed: " << pwstrDeviceId << std::endl;
		if (pManager) {
			pManager->PopulateDeviceLists();
		}
	} //DeviceRemoved

	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override {
	std:
		std::wcout << L"Default Device Changed: " << (flow == eCapture ? L"Input" : L"Output") << std::endl;
		if (pManager) {
			pManager->PopulateDeviceLists();
		}
	} //DefaultDeviceChanged

	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override {
		return S_OK;
	}

private:
	AudioDeviceManager *pManager;
	LONG refCount;
}; //IMMNotificationClient

//  设备管理器
AudioDeviceManager::AudioDeviceManager()
	: pEnumerator(nullptr), pNotificationClient(nullptr) {}

AudioDeviceManager::~AudioDeviceManager() {
	UnregisterDeviceNotifications();

	if (pEnumerator) {
		pEnumerator->Release();
		pEnumerator = nullptr;
	}
}

bool AudioDeviceManager::Initialize() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
	std:
		std::wcerr << L"COM initialize failed: " << _com_error(hr).ErrorMessage() << std::endl;
		return false;
	};

	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator),
		nullptr,
		CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void **) &pEnumerator
	);

	if (FAILED(hr)) {
	std:
		std::wcerr << L"Failed to create device enumerator: " << _com_error(hr).ErrorMessage() << std::endl;
		CoUninitialize();
		return false;
	}

	isInitialized = true;
	return true;
} //Initialize

bool AudioDeviceManager::PopulateDeviceLists() {
	if (!isInitialized) {
		std::wcerr << L"AudioDeviceManager not initialized" << std::endl;
		return false;
	}

	//  清空现有列表
	physicalInputs.clear();
	virtualInputs.clear();
	physicalOutputs.clear();
	virtualInputs.clear();

	//  枚举输入设备
	std::vector<AudioDevice> inputDevices;
	if (FAILED(EnumerateDevices(eCapture, inputDevices))) {
		return false;
	}

	//  枚举输出设备
	std::vector<AudioDevice> outputDevices;
	if (FAILED(EnumerateDevices(eRender, outputDevices))) {
		return false;
	}

	//  分类设备
	for (const auto &device: inputDevices) {
		if (device.type == DeviceType::Physical) {
			physicalInputs.push_back(device);
		} else {
			virtualInputs.push_back(device);
		}
	}

	for (const auto &device: outputDevices) {
		if (device.type == DeviceType::Physical) {
			physicalOutputs.push_back(device);
		} else {
			virtualOutputs.push_back(device);
		}
	}

	return true;
} //PopulateDeviceLists

HRESULT AudioDeviceManager::EnumerateDevices(EDataFlow dataFlow, std::vector<AudioDevice> &devices) {
	if (!pEnumerator) {
		return E_FAIL;
	}

	IMMDeviceCollection* pCollection = nullptr;
	HRESULT hr = pEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pCollection);
	if (FAILED(hr)) {
		std::wcerr << L"Failed to enumerate audio endpoints: " << _com_error(hr).ErrorMessage() << std::endl;
		return hr;
	}

	UINT count;
	hr = pCollection->GetCount(&count);
	if (FAILED(hr)) {
		pCollection->Release();
		return hr;
	}

	for (UINT i = 0; i < count; i++) {
		IMMDevice *pDevice = nullptr;
		hr = pCollection->Item(i, &pDevice);
		if (SUCCEEDED(hr)) {
			AudioDevice deviceInfo;

			//  获取设备ID
			LPWSTR pwszID = nullptr;
			pDevice->GetId(&pwszID);
			if (pwszID) {
				deviceInfo.id = pwszID;
				CoTaskMemFree(pwszID);
			}

			//  获取设备属性存储
			IPropertyStore *pProps = nullptr;
			hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
			if (SUCCEEDED(hr)) {
				//  获取设备友好名称
				PROPVARIANT varName;
				PropVariantInit(&varName);
				hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
				if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
					deviceInfo.name = varName.pwszVal;
				}
				PropVariantClear(&varName);

				//  获取设备枚举器名称
				PROPVARIANT varEnum;
				PropVariantInit(&varName);
				hr = pProps->GetValue(PKEY_Device_EnumeratorName, &varEnum);
				if (SUCCEEDED(hr) && varEnum.vt == VT_LPWSTR) {
					deviceInfo.enumerator = varEnum.pwszVal;
				}
				PropVariantClear(&varEnum);

				//  分类设备
				deviceInfo.type = ClassifyDevice(pDevice, pProps);

				//  设置设备方向
				deviceInfo.direction = (dataFlow == eCapture) ? DeviceDirection::Input : DeviceDirection::Output;

				// 检查是否为默认设备
				deviceInfo.isDefault = CheckIfDefaultDevice(pDevice, dataFlow);

				pProps->Release();
			}

			devices.push_back(deviceInfo);
			pDevice->Release();
		}
	}

	pCollection->Release();
	return S_OK;
}

DeviceType AudioDeviceManager::ClassifyDevice(IMMDevice *pDevice, IPropertyStore *pProps) {
	//  获取设备接口友好名称（更详细）
	PROPVARIANT varInterfaceName;
	PropVariantInit(&varInterfaceName);
	HRESULT hr = pProps->GetValue(PKEY_Device_Interface_FriendlyName, &varInterfaceName);

	std::wstring interfaceName;
	if (SUCCEEDED(hr) && varInterfaceName.vt == VT_LPWSTR) {
		interfaceName = varInterfaceName.pwszVal;
	}
	PropVariantClear(&varInterfaceName);

	//  获取设备枚举器名称
	PROPVARIANT varEnum;
	PropVariantInit(&varEnum);
	hr = pProps->GetValue(PKEY_Device_EnumeratorName, &varEnum);

	std::wstring enumeratorName;
	if (SUCCEEDED(hr) && varEnum.vt == VT_LPWSTR) {
		enumeratorName = varEnum.pwszVal;
	}
	PropVariantClear(&varEnum);

	//  转换为小写以便比较
	std::transform(interfaceName.begin(), interfaceName.end(), interfaceName.begin(), ::towlower);
	std::transform(enumeratorName.begin(), enumeratorName.end(), enumeratorName.begin(), ::tolower);

	//  检查已知的虚拟设备枚举器
	if (enumeratorName == L"swd" || enumeratorName == L"mmdevapi") {
		return DeviceType::Virtual;
	}

	//  检查已知的物理设备枚举器
	if (enumeratorName == L"pci" ||
		enumeratorName == L"usb" ||
		enumeratorName == L"hdaudio" ||
		enumeratorName == L"hdmi" ||
		enumeratorName == L"bluetooth" ||
		enumeratorName == L"bthenum") {
		return DeviceType::Physical;
	}

	//  检查接口名称中的关键字
	if (interfaceName

		.find(L"virtual") != std::wstring::npos ||
		interfaceName.find(L"voicemeeter") != std::wstring::npos ||
		interfaceName.find(L"vb_audio") != std::wstring::npos ||
		interfaceName.find(L"cable") != std::wstring::npos) {
		return DeviceType::Virtual;
	}

	//  默认情况下假设为物理设备
	return DeviceType::Physical;
}

bool AudioDeviceManager::CheckIfDefaultDevice(IMMDevice *pDevice, EDataFlow dataFlow) {
	if (!pEnumerator || !pDevice) return false;

	IMMDevice *pDefaultDevice = nullptr;
	HRESULT hr = pEnumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDefaultDevice);
	if (FAILED(hr)) {
		return false;
	}

	LPWSTR pwszDefaultID = nullptr;
	pDefaultDevice->GetId(&pwszDefaultID);

	LPWSTR pwszDeviceID = nullptr;
	pDevice->GetId(&pwszDeviceID);

	bool isDefault = ((pwszDefaultID && pwszDeviceID) && wcscmp(pwszDefaultID, pwszDeviceID) == 0);

	if (pwszDefaultID) CoTaskMemFree(pwszDefaultID);
	if (pwszDeviceID) CoTaskMemFree(pwszDeviceID);
	pDefaultDevice->Release();

	return isDefault;
}

bool AudioDeviceManager::RegisterDeviceNotifications() {
	if (!pEnumerator || !isInitialized) {
		return false;
	}

	pNotificationClient = new CMMNotificationClient(this);
	HRESULT hr = pEnumerator->RegisterEndpointNotificationCallback(pNotificationClient);

	if (FAILED(hr)) {
		delete pNotificationClient;
		pNotificationClient = nullptr;
		return false;
	}

	return true;
}

bool AudioDeviceManager::UnregisterDeviceNotifications() {
	if (!pEnumerator || !pNotificationClient) {
		return false;
	}

	HRESULT hr = pEnumerator->UnregisterEndpointNotificationCallback(pNotificationClient);
	if (SUCCEEDED(hr)) {
		pNotificationClient->Release();
		pNotificationClient = nullptr;
		return true;
	}

	return false;
}
