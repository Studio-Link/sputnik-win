#define _WIN32_WINNT 0x0A00 
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <propidl.h>
#include <propvarutil.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <ks.h>
#include <ksmedia.h>

#include "EnumBuiltinDevices.h"

#define PRECONDITION(a) if(!(a)) { return; }
#define PRECONDITION_RETURN(a, b) if(!(a)) { return (b); }

template<class T = IUnknown> void SafeRelease(T*& pUnknown)
{
    if(pUnknown != 0)
    {
        pUnknown->Release();
        pUnknown = 0;
    }
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

bool EnumBuiltinDevices_w64(const uint32_t deviceType, STUDIO_LINK_DEVICE_LIST* devices)
{
    PRECONDITION_RETURN(deviceType > 0, false);
    PRECONDITION_RETURN(devices != 0, false);

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if(SUCCEEDED(hr))
    {
        IMMDeviceEnumerator* pDeviceEnumerator = 0;
        hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 0, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
        if(SUCCEEDED(hr) && (pDeviceEnumerator != 0))
        {
            EDataFlow dataFlow = eAll;
            if(deviceType == STUDIO_LINK_OUTPUT_DEVICES)
            {
                dataFlow = eRender;
            }
            else if(deviceType == STUDIO_LINK_INPUT_DEVICES)
            {
                dataFlow = eCapture;
            }

            IMMDeviceCollection* pDeviceCollection = 0;
            hr = pDeviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATEMASK_ALL, &pDeviceCollection);
            if(SUCCEEDED(hr) && (pDeviceCollection != 0))
            {
                UINT deviceCount = 0;
                hr = pDeviceCollection->GetCount(&deviceCount);
                if(SUCCEEDED(hr))
                {
                    devices->deviceCount = static_cast<size_t>(deviceCount);
                    for(UINT i = 0; (i < deviceCount) && (i < STUDIO_LINK_MAX_DEVICE_COUNT); i++)
                    {
                        IMMDevice* pDevice = 0;
                        hr = pDeviceCollection->Item(i, &pDevice);
                        if(SUCCEEDED(hr) && (pDevice != 0))
                        {
                            IPropertyStore* pProperties = 0;
                            hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
                            if(SUCCEEDED(hr))
                            {
                                PROPVARIANT deviceNameProperty;
                                PropVariantInit(&deviceNameProperty);
                                hr = pProperties->GetValue(PKEY_Device_FriendlyName, &deviceNameProperty);
                                if(SUCCEEDED(hr))
                                {
                                    PWSTR deviceName = 0;
                                    hr = PropVariantToStringAlloc(deviceNameProperty, &deviceName);
                                    if(SUCCEEDED(hr))
                                    {
                                        memset(devices->devices[i].name, 0, STUDIO_LINK_DEVICE_NAME_LENGTH);
                                        size_t numOfCharsConverted = 0;
                                        const size_t deviceNameLength = wcslen(deviceName);
                                        wcstombs_s(&numOfCharsConverted, devices->devices[i].name, 
                                            STUDIO_LINK_DEVICE_NAME_LENGTH, deviceName, deviceNameLength);

                                        CoTaskMemFree(deviceName);
                                        deviceName = 0;
                                    }

                                    PropVariantClear(&deviceNameProperty);
                                }

                                PROPVARIANT deviceFormatProperty;
                                PropVariantInit(&deviceFormatProperty);
                                hr = pProperties->GetValue(PKEY_AudioEngine_DeviceFormat, &deviceFormatProperty);
                                if(SUCCEEDED(hr) && (VT_BLOB == deviceFormatProperty.vt))
                                {
                                    WAVEFORMATEX* deviceFormat = (WAVEFORMATEX*)deviceFormatProperty.blob.pBlobData;
                                    devices->devices[i].channelCount = deviceFormat->nChannels;
                                    devices->devices[i].sampleRate = deviceFormat->nSamplesPerSec;

                                    if((WAVE_FORMAT_EXTENSIBLE == deviceFormat->wFormatTag) && (deviceFormat->cbSize >= 22))
                                    {
                                        //WAVEFORMATEXTENSIBLE* extensibleDeviceFormat = (WAVEFORMATEXTENSIBLE*)deviceFormat;
                                    }

                                    PropVariantClear(&deviceFormatProperty);
                                }

                                SafeRelease(pProperties);
                            }

                            SafeRelease(pDevice);
                        }
                    }
                }

                SafeRelease(pDeviceCollection);
            }

            SafeRelease(pDeviceEnumerator);
        }

        CoUninitialize();
    }

    return 0;
}

