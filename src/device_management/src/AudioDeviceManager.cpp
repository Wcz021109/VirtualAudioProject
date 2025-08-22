//  设备管理实现文件
#include "AudioDeviceManager.h"
#include <iostream>
#include <comdef.h>

#pragma comment(lib, "Ole32.lib")

//  设备通知客户端
class CMMNotificationClient : public IMMNotificationClient{
public:
    CMMNotificationClient(AudioDeviceManager* pManager) :
        pManager(pManager), result(1) {}

    //  IUnknown方法
    ULONG STDMETHODCALLTYPE AddRef() override {
        return InterlockedIncrement(&result);
    }//IUnknown.Addref

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG ulRef = InterlockedDecrement(&refcount);
        if (ulRef == 0) {
            delete this;
        }
        return ulRef;
    }//IUnknown.Release

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvInterface) override {
        if (riid == IID_IUnknown) {
        AddRef();
        *ppvInterface = (IUnknown*)this;
        }else if (riid == __uuidof(IMMNotificationClient)) {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }else {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    //  IMMNotificationClient方法
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged (LPCWSTR pwstrDeviceId, DWORD dwNewState) override {
        std:wcout << L"Device state changed: " << pwstrDeviceId << std::endl;
        if (pManager) {
            pManager->PopulateDeviceLists();
        }
        return S_OK;
    }//DeviceStateChange

    HRESULT STDMETHODCALLTYPE OnDeviceAdded (LPCWSTR pwstrDeviceId) override {
        std:wcout << L"Device Added: " << pwstrDeviceId << std::endl;
        if (pManager) {
            pManager->PopulateDeviceLists();
        }
        return S_OK;
    }//DeviceAdded

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved (LPCWSTR pwstrDeviceId) override {
        std:wcout << L"Device Removed: " << pwstrDeviceId << std::endl;
        if (pManager) {
            pManager->PopulateDeviceLists();
        }
    }//DeviceRemoved

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override {
        std:wcout << L"Default Device Changed: " << (flow == eCapture ? L"Input" : L"Output") << std::endl;
        if (pManager) {
            pManager->PopulateDeviceLists();
        }
    }//DefaultDeviceChanged

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged (LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override {
        return S_OK;
    }

private:
    AudioDeviceManager* pManager;
    LONG refCount;
};//IMMNotificationClient

//  设备管理器
AudioDeviceManager::AudioDeviceManager()
    : pEnumber(nullptr), pNotificationClient(nullptr) {}

AudioDeviceManager::~AudioDeviceManager() {
    UnregisterDeviceNotification();

    if (pEnumeration) {
        pEnumeration->Release();
        pEnumeration = nullptr;
    }
}

bool AudioDeviceManager::Initialize() {
    HRESULT hr = CoInitialize(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std:wcerr << L"COM initialize failed: " << _com_error(hr) << std::endl;
        return false;
    };

    hr = CoCreateInstance(
        _uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        _uuid0f(IMMDeviceEnumerator),
        (void**)&pEnumerator
    );

    if (FAILED(hr)) {
        std:wcerr << L"Failed to create device enumerator: " << _com_error(hr).ErrorMessage() << std::endl;
        CoUninitialize();
        return false;
    }

    isInitialized = true;
    return true;
}//Initallize

bool AudioDeviceManager::PopulateDeviceLists() {
    if (!isInitialized) {
        std::wcerr <<  L"AudioDeviceManager not initialized" << std::endl;
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
    if (FAILED(EnumerateDevices(eOutput, outputDevices))) {
        return false;
    }

    //  分类设备
    for (const auto& device : inputDevices) {
        if (device.type == DeviceType::Physical) {
            physicalInputs.push_back(device);
        } else {
            virtualInputs.push_back(device);
        }
    }

    for (const auto& device : outputDevices) {
        if (device.type == DeviceType::Physical) {
            physicalOutputs.push_back(device);
        } else {
            virtualOutputs.push_back(device);
        }
    }

    return true;
}//PopulateDeviceLists

HRESULT AudioDeviceManager::EnumerateDevices(EDataFlow DataFlow, std::vector<AudioDevice>& devices) {
    if (!pEnumerator) {
        return E_FAIL;
    }

    IMMDeviceEnumerator* pCollection = nullptr;
    HResult hr = pEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        std:wcerr << L"Failed to enumerate audio endpoints: " << _com_error(hr).ErrorMessage() << std::endl;
        return hr;
    }

    UINT count;
    hr = pCollection->GetCount(&count);
    if (FAILED(hr)) {
        pCollection->Release();
        return hr;
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDevice = nullptr;
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
            IPropertyStore* pProps = nullptr;
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
                //  分类设备
                //  设置设备方向  
                // 检查是否为默认设备
            }
        }
    }
}

