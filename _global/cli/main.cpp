#include <iostream>
#include "../inc/StudioLink.h"

int main(int argc, const char * argv[])
{
    STUDIO_LINK_DEVICE_TYPE deviceType = HEADPHONE;
    STUDIO_LINK_DEVICE_LIST devices = {0};
    
    devices.version = 1;
    const bool result = StudioLinkEnumBuiltinDevices(deviceType, &devices);
    if(result == true)
    {
        for(size_t i = 0; i < devices.deviceCount; i++)
        {
            std::cout << "(" << (i + 1) << ") " << devices.devices[i].name << std::endl;
            std::cout << "\tSample rate: " << devices.devices[i].sampleRate << std::endl;
            std::cout << "\tChannels:    " << devices.devices[i].channelCount << std::endl;
        }
    }
    
    return 0;
}
