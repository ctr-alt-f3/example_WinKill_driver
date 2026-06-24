#include <ntifs.h>
#include <wdf.h>
#define MAIN_IOCTL_CODE CTL_CODE(0x00000022,0x800,0,0)
#define KILL_IOCTL_CODE CTL_CODE(0x00000022,0x801,0,0)

DRIVER_INITIALIZE DriverEntry_f; 
EVT_WDF_DRIVER_UNLOAD DriverUnload_f;
EVT_WDF_OBJECT_CONTEXT_DESTROY Destroy_f;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  EvtIoDeviceControl_f; 


VOID kill_f(char* input);

VOID DriverUnload_f(WDFDRIVER Driver) {
	UNREFERENCED_PARAMETER(Driver);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "VOLATUS: unLoaded\n"));
	return;
}
VOID Destroy_f(WDFOBJECT Object) {
	UNREFERENCED_PARAMETER(Object);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "VOLATUS: DESTROYED\n"));
	return;
}


NTSTATUS DriverEntry(PDRIVER_OBJECT drv_obj, PUNICODE_STRING reg_path) {
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "VOLATUS AD ASTRA\n"));

	WDF_DRIVER_CONFIG config;
	WDFDRIVER drv;
	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = DriverUnload_f;

	WDF_OBJECT_ATTRIBUTES attrs;
	WDF_OBJECT_ATTRIBUTES_INIT(&attrs);
	attrs.EvtDestroyCallback = Destroy_f;
	NTSTATUS stat = WdfDriverCreate(drv_obj, reg_path, &attrs, &config, &drv);
	if (!NT_SUCCESS(stat)) {
		return stat;
	}
	DECLARE_CONST_UNICODE_STRING(DevNazwaKrnl, L"\\Device\\VolatusDRV");
	DECLARE_CONST_UNICODE_STRING(DevNazwaUsr, L"\\DosDevices\\VolatusDRV");

	PWDFDEVICE_INIT devInit = WdfControlDeviceInitAllocate(drv, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
	if (devInit == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	stat = WdfDeviceInitAssignName(devInit, &DevNazwaKrnl);
	if (!NT_SUCCESS(stat)) {
		return stat;
	}
	WDFDEVICE DevKrnl;
	stat = WdfDeviceCreate(&devInit, WDF_NO_OBJECT_ATTRIBUTES, &DevKrnl);
	if (!NT_SUCCESS(stat)) {
		return stat;
	}
	stat = WdfDeviceCreateSymbolicLink(DevKrnl, &DevNazwaUsr);
	//

	WDF_IO_QUEUE_CONFIG qconf; 
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&qconf, WdfIoQueueDispatchSequential);
	qconf.EvtIoDeviceControl = EvtIoDeviceControl_f;
	stat = WdfIoQueueCreate(DevKrnl, &qconf, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
	if (!NT_SUCCESS(stat)) {
		return stat;
	}
	//
	WdfControlFinishInitializing(DevKrnl);
	return stat;
}

VOID EvtIoDeviceControl_f(
	_In_
	WDFQUEUE Queue,
	_In_
	WDFREQUEST Request,
	_In_
	size_t OutputBufferLength,
	_In_
	size_t InputBufferLength,
	_In_
	ULONG IoControlCode
) {
	UNREFERENCED_PARAMETER(Queue);
	NTSTATUS stat;
	void* input;
	void* output;
	if (IoControlCode != MAIN_IOCTL_CODE && IoControlCode != KILL_IOCTL_CODE) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV: podano zly kod IOCTL\n"));
	} 
	if (IoControlCode == KILL_IOCTL_CODE) {
		input = NULL;
		output = NULL;

		stat = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &input, NULL);
		if (!NT_SUCCESS(stat) || input == NULL) {
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: blad przy braniu input buffera\n"));
			WdfRequestComplete(Request, stat);
			return;
		}

		stat = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &output, NULL);
		if (!NT_SUCCESS(stat) || output == NULL) {
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: blad przy braniu out buffera\n"));
			WdfRequestComplete(Request, stat);
			return;
		}

		((char*)input)[InputBufferLength - 1] = '\0';

		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "VolatusDRV: killing PID: %s\n", (char*)input));
		kill_f((char*)input);

		strcpy_s(output, OutputBufferLength, "VolatusDRV_Kill: VOLATUS AD ASTRA\n");

		size_t bytesRet = strlen("VolatusDRV_Kill: VOLATUS AD ASTRA\n") + 1;
		stat = STATUS_SUCCESS;
		WdfRequestCompleteWithInformation(Request, stat, bytesRet);
		return;
	}
	stat = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &input, NULL);
	if(!NT_SUCCESS(stat) || input == NULL){
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV: blad przy braniu input buffera\n"));
	}

	stat = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &output, NULL);
	if (!NT_SUCCESS(stat) || input == NULL) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV: blad przy braniu out buffera\n"));
	}
	strcpy_s(output, OutputBufferLength, "VolatusDRV: VOLATUS AD ASTRA\n");
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "VolatusDRV: received: %s\n", (char*)input));
	size_t bytesRet = strlen("VolatusDRV: VOLATUS AD ASTRA\n") + 1;
	stat = STATUS_SUCCESS;
	WdfRequestCompleteWithInformation(Request, stat, bytesRet);
}


VOID kill_f(
	char* input
) {

	HANDLE a = 0;
	ULONG b;
	NTSTATUS stat;
	stat = RtlCharToInteger(input, 10, &b);
	if (!(NT_SUCCESS(stat))) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: blad przy RtlCharToInt\n"));
		return;
	}
	PEPROCESS proc;
	 stat = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)b, &proc);
	if (!(NT_SUCCESS(stat))) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: PID nie istnieje\n"));
		return;
	}
	stat = ObOpenObjectByPointer(proc, OBJ_KERNEL_HANDLE, NULL, 0x0001, NULL, KernelMode, &a);
	if (!(NT_SUCCESS(stat))) {
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: blad przy funkcji OpenObjByPtr, nic nie ubito\n"));
		ObDereferenceObject(proc);

		return;
	}
	ZwTerminateProcess(a, 0);
	ZwClose(a);
	ObDereferenceObject(proc);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "VolatusDRV_Kill: kiLled\n"));
	return;
}