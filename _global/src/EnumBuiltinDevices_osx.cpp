#include <CoreAudio/CoreAudio.h>
#include "StudioLink.h"
#include "EnumBuiltinDevices.h"

#define PRECONDITION_RETURN(a, b) if(!(a)) { return (b); }

bool EnumBuiltinDevices_osx(const STUDIO_LINK_DEVICE_TYPE deviceType, STUDIO_LINK_DEVICE_LIST* devices)
{
    PRECONDITION_RETURN(deviceType != INVALID_DEVICE_TYPE, false);
    PRECONDITION_RETURN(devices != 0, false);
    
    AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal;
    if(deviceType == MICROPHONE)
    {
        scope = kAudioObjectPropertyScopeInput;
    }
    else
    {
        scope = kAudioObjectPropertyScopeOutput;
    }
    
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        scope,
        kAudioObjectPropertyElementMaster
    };
    
    UInt32 dataSize = 0;
    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                                     &propertyAddress,
                                                     0, NULL, &dataSize);
    if(kAudioHardwareNoError == status)
    {
        size_t deviceIdCount = dataSize / sizeof(AudioDeviceID);
        AudioDeviceID deviceIds[deviceIdCount];
        status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize, deviceIds);
        if(kAudioHardwareNoError == status)
        {
            for(UInt32 i = 0; (i < deviceIdCount) && (i < STUDIO_LINK_MAX_DEVICE_COUNT); i++)
            {
                CFStringRef deviceName = 0;
                dataSize = sizeof(deviceName);
                propertyAddress.mSelector = kAudioDevicePropertyDeviceNameCFString;
                status = AudioObjectGetPropertyData(deviceIds[i],
                                                    &propertyAddress,
                                                    0, NULL, &dataSize, &deviceName);
                if(kAudioHardwareNoError == status)
                {
                    memset(devices->devices[i].name, 0, STUDIO_LINK_DEVICE_NAME_LENGTH * sizeof(char));
                    strcpy(devices->devices[i].name, CFStringGetCStringPtr(deviceName, kCFStringEncodingUTF8));
                    CFRelease(deviceName);
                }

                dataSize = 0;
                propertyAddress.mScope = scope,
                propertyAddress.mSelector = kAudioDevicePropertyStreams;
                status = AudioObjectGetPropertyDataSize(deviceIds[i],
                                                        &propertyAddress,
                                                        0,
                                                        0,
                                                        &dataSize);
                if(kAudioHardwareNoError == status)
                {
                    const size_t streamIdCount = dataSize / sizeof(AudioStreamID);
                    AudioStreamID streamIds[streamIdCount];
                    propertyAddress.mSelector = kAudioDevicePropertyStreams;
                    status = AudioObjectGetPropertyData(deviceIds[i],
                                                        &propertyAddress,
                                                        0,
                                                        0,
                                                        &dataSize,
                                                        &streamIds);
                    if(kAudioHardwareNoError == status)
                    {
                        // FIXME: handle multiple streams
                        if(streamIdCount > 0)
                        {
                            AudioStreamID streamId = streamIds[0];
                            AudioStreamBasicDescription description = {0};
                            dataSize = sizeof(AudioStreamBasicDescription);
                            propertyAddress.mSelector = kAudioStreamPropertyVirtualFormat;
                            status = AudioObjectGetPropertyData(streamId,
                                                                &propertyAddress,
                                                                0,
                                                                0,
                                                                &dataSize,
                                                                &description);
                            if(kAudioHardwareNoError == status)
                            {
                                devices->devices[i].sampleRate = static_cast<double>(description.mSampleRate);
                                devices->devices[i].channelCount = static_cast<uint32_t>(description.mChannelsPerFrame);
                            }
                        }
                    }
                }
            }
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

