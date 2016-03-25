#include <CoreAudio/CoreAudio.h>
#include "StudioLink.h"
#include "EnumBuiltinDevices.h"

bool EnumBuiltinDevices_osx(const uint32_t deviceType, STUDIO_LINK_DEVICE_LIST* devices)
{
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
    
    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                     &propertyAddress,
                                                     0, NULL, &dataSize);
    if(kAudioHardwareNoError == status)
    {
        UInt32 deviceCount = dataSize / sizeof(AudioDeviceID);
        
        AudioDeviceID* outputDevices = new AudioDeviceID[deviceCount];
        if(outputDevices != 0)
        {
            status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                &propertyAddress,
                                                0, NULL, &dataSize, outputDevices);
            if(kAudioHardwareNoError == status)
            {
                for(UInt32 i = 0; (i < deviceCount) && (i < STUDIO_LINK_MAX_DEVICE_COUNT); i++)
                {
                    bool selectDevice = false;
                    dataSize = 0;
                    propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
                    status = AudioObjectGetPropertyDataSize(outputDevices[i], &propertyAddress, 0, NULL, &dataSize);
                    if(kAudioHardwareNoError == status)
                    {
                        AudioBufferList* bufferList = static_cast<AudioBufferList *>(malloc(dataSize));
                        if(bufferList != NULL)
                        {
                            status = AudioObjectGetPropertyData(outputDevices[i], &propertyAddress, 0, NULL, &dataSize, bufferList);
                            if(kAudioHardwareNoError == status)
                            {
                                if(((0 != bufferList->mNumberBuffers) && (deviceType & STUDIO_LINK_OUTPUT_DEVICES)) ||
                                   ((0 == bufferList->mNumberBuffers) && (deviceType & STUDIO_LINK_INPUT_DEVICES)))
                                {
                                    selectDevice = true;
                                }
                            }
                            
                            free(bufferList);
                        }
                    }
                    
                    if(selectDevice == true)
                    {
                        CFStringRef deviceName = 0;
                        dataSize = sizeof(deviceName);
                        propertyAddress.mSelector = kAudioDevicePropertyDeviceNameCFString;
                        status = AudioObjectGetPropertyData(outputDevices[i],
                                                            &propertyAddress,
                                                            0, NULL, &dataSize, &deviceName);
                        if(kAudioHardwareNoError == status)
                        {
                            memset(devices->devices[i].name, 0, STUDIO_LINK_DEVICE_NAME_LENGTH * sizeof(char));
                            strcpy(devices->devices[i].name, CFStringGetCStringPtr(deviceName, kCFStringEncodingUTF8));
                            CFRelease(deviceName);
                        }
                    }
                }
            }
            
            delete [] outputDevices;
        }
    }
    
    return false;
}

bool EnumBuiltinDevicesInitialize_osx()
{
    return false;
}

void EnumBuiltinDevicesUninitialize_osx()
{
}

