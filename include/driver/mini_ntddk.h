#ifndef MINI_NTDDK_H
#define MINI_NTDDK_H

#include <windows.h>

// --- Basic Types ---
typedef LONG NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#endif
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef PVOID PEPROCESS;

// --- DDK Constants ---
#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN             0x00000022
#endif
#ifndef FILE_DEVICE_SECURE_OPEN
#define FILE_DEVICE_SECURE_OPEN         0x00000100
#endif
#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED                 0
#endif
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS                 0
#endif

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#define IOCTL_HIDE_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

// --- Major Function Codes ---
#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

// --- Structures ---
typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT;
typedef struct _IRP *PIRP;

typedef NTSTATUS (NTAPI *PDRIVER_DISPATCH)(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
);

typedef void (NTAPI *PDRIVER_UNLOAD)(
    PDRIVER_OBJECT DriverObject
);

typedef struct _DRIVER_OBJECT {
    short Type;
    short Size;
    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;
    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    PVOID DriverExtension;
    UNICODE_STRING DriverName;
    PUNICODE_STRING HardwareDatabase;
    PVOID FastIoDispatch;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
    void (NTAPI *DriverUnload)(PDRIVER_OBJECT);
} DRIVER_OBJECT;

typedef struct _DEVICE_OBJECT {
    short Type;
    unsigned short Size;
    LONG ReferenceCount;
    PDRIVER_OBJECT DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    PIRP CurrentIrp;
    ULONG Flags;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
} DEVICE_OBJECT;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _IRP {
    short Type;
    unsigned short Size;
    PVOID MdlAddress;
    ULONG Flags;
    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;
    LIST_ENTRY ThreadListEntry;
    IO_STATUS_BLOCK IoStatus;
    PVOID CurrentStackLocation; 
} IRP;

typedef struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;
    union {
        struct {
            ULONG OutputBufferLength;
            ULONG InputBufferLength;
            ULONG IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;
    } Parameters;
    PDEVICE_OBJECT DeviceObject;
    PVOID FileObject;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

// --- Prototypes ---
#ifdef __cplusplus
extern "C" {
#endif
    NTSTATUS NTAPI IoCreateDevice(PDRIVER_OBJECT, ULONG, PUNICODE_STRING, DEVICE_TYPE, ULONG, BOOLEAN, PDEVICE_OBJECT*);
    NTSTATUS NTAPI IoCreateSymbolicLink(PUNICODE_STRING, PUNICODE_STRING);
    NTSTATUS NTAPI IoDeleteSymbolicLink(PUNICODE_STRING);
    void NTAPI IoDeleteDevice(PDEVICE_OBJECT);
    void NTAPI IoCompleteRequest(PIRP, CCHAR);
    NTSTATUS NTAPI PsLookupProcessByProcessId(HANDLE, PEPROCESS*);
    void NTAPI ObDereferenceObject(PVOID);
    void NTAPI RtlInitUnicodeString(PUNICODE_STRING, PCWSTR);
    ULONG NTAPI DbgPrint(PCSTR Format, ...);
    #define IO_NO_INCREMENT 0
#ifdef __cplusplus
}
#endif

#endif // MINI_NTDDK_H
