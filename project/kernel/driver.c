#include <ntddk.h>
#include <wdmsec.h>
#define QUERY_VERSION CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS)
typedef struct { ULONG Size; ULONG Abi; } BRIDGE_VERSION;
C_ASSERT(sizeof(BRIDGE_VERSION) == 8);
static const GUID DeviceClass = {0x651fc915,0xa6c8,0x4b91,{0x91,0xb6,0x46,0x85,0xcd,0x12,0x79,0x36}};
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD BridgeUnload;
DRIVER_DISPATCH BridgeUnsupported;
DRIVER_DISPATCH BridgeOpenClose;
DRIVER_DISPATCH BridgeControl;
static NTSTATUS Complete(PIRP irp, NTSTATUS status, ULONG_PTR bytes) {
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = bytes;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}
NTSTATUS BridgeUnsupported(PDEVICE_OBJECT device, PIRP irp) {
    UNREFERENCED_PARAMETER(device);
    return Complete(irp, STATUS_INVALID_DEVICE_REQUEST, 0);
}
NTSTATUS BridgeOpenClose(PDEVICE_OBJECT device, PIRP irp) {
    UNREFERENCED_PARAMETER(device);
    return Complete(irp, STATUS_SUCCESS, 0);
}
NTSTATUS BridgeControl(PDEVICE_OBJECT device, PIRP irp) {
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
    BRIDGE_VERSION* result;
    UNREFERENCED_PARAMETER(device);
    if (stack->Parameters.DeviceIoControl.IoControlCode != QUERY_VERSION)
        return Complete(irp, STATUS_INVALID_DEVICE_REQUEST, 0);
    if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(BRIDGE_VERSION))
        return Complete(irp, STATUS_BUFFER_TOO_SMALL, 0);
    result = (BRIDGE_VERSION*)irp->AssociatedIrp.SystemBuffer;
    if (!result) return Complete(irp, STATUS_INVALID_PARAMETER, 0);
    result->Size = sizeof(*result);
    result->Abi = 1;
    return Complete(irp, STATUS_SUCCESS, sizeof(*result));
}
VOID BridgeUnload(PDRIVER_OBJECT driver) {
    UNICODE_STRING link = RTL_CONSTANT_STRING(L"\\DosDevices\\RustInternalBridge");
    IoDeleteSymbolicLink(&link);
    if (driver->DeviceObject) IoDeleteDevice(driver->DeviceObject);
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING registry) {
    UNICODE_STRING name = RTL_CONSTANT_STRING(L"\\Device\\RustInternalBridge");
    UNICODE_STRING link = RTL_CONSTANT_STRING(L"\\DosDevices\\RustInternalBridge");
    UNICODE_STRING acl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");
    PDEVICE_OBJECT device = NULL;
    NTSTATUS status;
    ULONG i;
    UNREFERENCED_PARAMETER(registry);
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i) driver->MajorFunction[i] = BridgeUnsupported;
    driver->MajorFunction[IRP_MJ_CREATE] = BridgeOpenClose;
    driver->MajorFunction[IRP_MJ_CLOSE] = BridgeOpenClose;
    driver->MajorFunction[IRP_MJ_CLEANUP] = BridgeOpenClose;
    driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BridgeControl;
    driver->DriverUnload = BridgeUnload;
    status = IoCreateDeviceSecure(driver, 0, &name, FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN, FALSE, &acl, &DeviceClass, &device);
    if (!NT_SUCCESS(status)) return status;
    status = IoCreateSymbolicLink(&link, &name);
    if (!NT_SUCCESS(status)) { IoDeleteDevice(device); return status; }
    device->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}
