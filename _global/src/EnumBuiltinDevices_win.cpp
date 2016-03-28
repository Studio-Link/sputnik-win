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
    PRECONDITION_RETURN(deviceType != INVALID_DEVICE_TYPE, false);
    PRECONDITION_RETURN(devices != 0, false);

    bool result = false;

    HRESULT hr = CoInitialize(0);
    if(SUCCEEDED(hr))
    {
        IMMDeviceEnumerator* pDeviceEnumerator = 0;
        hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 0, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
        if(SUCCEEDED(hr) && (pDeviceEnumerator != 0))
        {
            EDataFlow dataFlow = eAll;
            if(deviceType == HEADPHONE)
            {
                dataFlow = eRender;
            }
            else if(deviceType == MICROPHONE)
            {
                dataFlow = eCapture;
            }

            IMMDeviceCollection* pDeviceCollection = 0;
            hr = pDeviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pDeviceCollection);
            if(SUCCEEDED(hr) && (pDeviceCollection != 0))
            {
                UINT deviceCount = 0;
                hr = pDeviceCollection->GetCount(&deviceCount);
                if(SUCCEEDED(hr))
                {
                    size_t foundDevices = 0;
                    for(UINT i = 0; (i < deviceCount) && (foundDevices < deviceCount); i++)
                    {
                        IMMDevice* pDevice = 0;
                        hr = pDeviceCollection->Item(i, &pDevice);
                        if(SUCCEEDED(hr) && (pDevice != 0))
                        {
                            IPropertyStore* pProperties = 0;
                            hr = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
                            if(SUCCEEDED(hr))
                            {
                                PROPVARIANT deviceFormatProperty;
                                PropVariantInit(&deviceFormatProperty);
                                hr = pProperties->GetValue(PKEY_AudioEngine_DeviceFormat, &deviceFormatProperty);
                                if(SUCCEEDED(hr) && (VT_BLOB == deviceFormatProperty.vt))
                                {
                                    WAVEFORMATEX* deviceFormat = (WAVEFORMATEX*)deviceFormatProperty.blob.pBlobData;
                                    devices->devices[foundDevices].channelCount = deviceFormat->nChannels;
                                    devices->devices[foundDevices].sampleRate = deviceFormat->nSamplesPerSec;

                                    if((WAVE_FORMAT_EXTENSIBLE == deviceFormat->wFormatTag) && (deviceFormat->cbSize >= 22))
                                    {
                                        //WAVEFORMATEXTENSIBLE* extensibleDeviceFormat = (WAVEFORMATEXTENSIBLE*)deviceFormat;
                                    }

                                    PropVariantClear(&deviceFormatProperty);

                                    PROPVARIANT deviceNameProperty;
                                    PropVariantInit(&deviceNameProperty);
                                    hr = pProperties->GetValue(PKEY_Device_FriendlyName, &deviceNameProperty);
                                    if(SUCCEEDED(hr))
                                    {
                                        PWSTR deviceName = 0;
                                        hr = PropVariantToStringAlloc(deviceNameProperty, &deviceName);
                                        if(SUCCEEDED(hr))
                                        {
                                            memset(devices->devices[foundDevices].name, 0, STUDIO_LINK_DEVICE_NAME_LENGTH);
                                            size_t numOfCharsConverted = 0;
                                            const size_t deviceNameLength = wcslen(deviceName);
                                            wcstombs_s(&numOfCharsConverted, devices->devices[foundDevices].name,
                                                STUDIO_LINK_DEVICE_NAME_LENGTH, deviceName, deviceNameLength);

                                            foundDevices++;

                                            CoTaskMemFree(deviceName);
                                            deviceName = 0;
                                        }

                                        PropVariantClear(&deviceNameProperty);
                                    }
                                }

                                SafeRelease(pProperties);
                            }

                            SafeRelease(pDevice);
                        }
                    }

                    devices->deviceCount = foundDevices;
                }

                SafeRelease(pDeviceCollection);
            }

            SafeRelease(pDeviceEnumerator);
        }

        CoUninitialize();
    }

    if(devices->deviceCount > 0)
    {
        result = true;
    }

    return result;
}

