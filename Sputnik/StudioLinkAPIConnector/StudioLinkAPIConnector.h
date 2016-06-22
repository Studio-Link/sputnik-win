#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace StudioLinkAPI {

    public ref class Device
    {
    public:
        Device(const int channels, const double sampleRate, const String^ name);

        inline int Channels(); 
        inline double SampleRate();
        inline const String^ Name();

    private:
        const int channels_;
        const double sampleRate_;
        const String^ name_;
    };

    inline int Device::Channels()
    {
        return channels_;
    }

    inline double Device::SampleRate()
    {
        return sampleRate_;
    }

    inline const String^ Device::Name()
    {
        return name_;
    }

    public enum DeviceType {
        INPUT_DEVICE,
        OUTPUT_DEVICE
    };

	public ref class Connector
	{
    public:
        IEnumerable<Device^>^ EnumBuiltinDevices(const DeviceType type);
	};
}
