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
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged (LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override {
        return S_OK;
    }

private:
    AudioDeviceManager* pManager;
    LONG refCount;
};//IMMNotificationClient

//  设备管理器