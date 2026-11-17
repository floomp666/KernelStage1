#pragma once
#include <Windows.h>
namespace kernel_bridge {
struct Version { DWORD size; DWORD abi; };
static_assert(sizeof(Version) == 8, "Protocol size mismatch");
inline bool query_version(Version& version) {
    version = {};
    HANDLE device = CreateFileW(L"\\\\.\\RustInternalBridge", GENERIC_READ,
        0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (device == INVALID_HANDLE_VALUE) return false;
    DWORD returned = 0;
    const DWORD ioctl = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS);
    BOOL ok = DeviceIoControl(device, ioctl, nullptr, 0, &version,
        sizeof(version), &returned, nullptr);
    CloseHandle(device);
    return ok && returned == sizeof(version) && version.size == sizeof(version) && version.abi == 1;
}
}
