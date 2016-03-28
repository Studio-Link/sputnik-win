#include <CoreAudio/CoreAudio.h>
#include "StudioLink.h"
#include "EnumBuiltinDevices.h"

#define PRECONDITION_RETURN(a, b) if(!(a)) { return (b); }

bool EnumBuiltinDevices_osx(const STUDIO_LINK_DEVICE_TYPE deviceType, STUDIO_LINK_DEVICE_LIST* devices)
{
    PRECONDITION_RETURN(deviceType != INVALID_DEVICE_TYPE, false);
    PRECONDITION_RETURN(devices != 0, false);
    
    bool result = false;
    
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
            size_t foundDevices = 0;
            for(UInt32 i = 0; (i < deviceIdCount) && (foundDevices < deviceIdCount); i++)
            {
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
                    if(streamIdCount > 0)
                    {
                        AudioStreamID streamIds[streamIdCount];
                        status = AudioObjectGetPropertyData(deviceIds[i],
                                                            &propertyAddress,
                                                            0,
                                                            0,
                                                            &dataSize,
                                                            &streamIds);
                        if(kAudioHardwareNoError == status)
                        {
                        // FIXME: handle multiple streams
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
                                CFStringRef deviceName = 0;
                                dataSize = sizeof(deviceName);
                                propertyAddress.mSelector = kAudioDevicePropertyDeviceNameCFString;
                                status = AudioObjectGetPropertyData(deviceIds[i],
                                                                    &propertyAddress,
                                                                    0, NULL, &dataSize, &deviceName);
                                if(kAudioHardwareNoError == status)
                                {
                                    if(CFStringGetLength(deviceName) > 0)
                                    {
                                        memset(devices->devices[foundDevices].name, 0, STUDIO_LINK_DEVICE_NAME_LENGTH * sizeof(char));
                                        strcpy(devices->devices[foundDevices].name, CFStringGetCStringPtr(deviceName, kCFStringEncodingUTF8));

                                        devices->devices[foundDevices].sampleRate = static_cast<double>(description.mSampleRate);
                                        devices->devices[foundDevices].channelCount = static_cast<uint32_t>(description.mChannelsPerFrame);

                                        foundDevices++;
                                    }
                                    
                                    CFRelease(deviceName);
                                }
                            }
                            
                        }
                    }
                }
            }
            
            devices->deviceCount = foundDevices;
        }
    }
    
    if(devices->deviceCount > 0)
    {
        result = true;
    }
    
    return result;
}

