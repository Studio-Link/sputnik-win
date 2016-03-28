#include "stdafx.h"
#include "StudioLinkAPIConnector.h"
#include "StudioLink.h"

namespace StudioLinkAPI {

Device::Device(const int channels, const double sampleRate, const String^ name) :
    channels_(channels), sampleRate_(sampleRate), name_(name)
{
}

IEnumerable<Device^>^ Connector::EnumBuiltinDevices(const DeviceType type)
{
    List<Device^>^ result = gcnew List<Device^>();

    STUDIO_LINK_DEVICE_TYPE deviceType = (type == INPUT_DEVICE) ? MICROPHONE : HEADPHONE;
    STUDIO_LINK_DEVICE_LIST devices = {0};
    const bool succeeded = StudioLinkEnumBuiltinDevices(deviceType, &devices);
    if(succeeded == true)
    {
        const size_t deviceCount = devices.deviceCount;
        if(deviceCount > 0)
        {
            for(size_t i = 0; i < deviceCount; i++)
            {
                Device^ device = gcnew Device {
                    devices.devices[i].channelCount,
                    devices.devices[i].sampleRate,
                    gcnew String(devices.devices[i].name)
                };

                result->Add(device);
            }
        }
    }

    return result;
}

}

