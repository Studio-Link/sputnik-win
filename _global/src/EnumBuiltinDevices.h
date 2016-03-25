#ifndef __STUDIO_LINK_ENUM_BUILTIN_DEVICES_H_INCL__
#define __STUDIO_LINK_ENUM_BUILTIN_DEVICES_H_INCL__

#include "StudioLink.h"

#ifdef _WIN64
bool EnumBuiltinDevices_w64(const uint32_t deviceType, STUDIO_LINK_DEVICE_LIST* devices);

bool EnumBuiltinDevicesInitialize_w64();

void EnumBuiltinDevicesUninitialize_w64();
#endif // #ifdef _WIN64

#ifdef _WIN64 
#define EnumBuiltinDevices EnumBuiltinDevices_w64
#define EnumBuiltinDevicesInitialize() EnumBuiltinDevicesInitialize_w64
#define EnumBuiltinDevicesUninitialize() EnumBuiltinDevicesUninitialize_w64
#endif // #ifndef _WIN64

#endif // #ifndef __STUDIO_LINK_ENUM_BUILTIN_DEVICES_H_INCL__
