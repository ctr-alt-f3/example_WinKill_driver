#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>


#define MAIN_IOCTL_CODE CTL_CODE(0x00000022,0x800,0,0)
#define KILL_IOCTL_CODE CTL_CODE(0x00000022,0x801,0,0)

int main() {
	HANDLE dev = CreateFile(L"\\\\.\\VolatusDRV", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dev == INVALID_HANDLE_VALUE) {
		printf("ERR: otwarcie urzadzenia dostalo wylewu\n");
		return 2137;
	}


	char* out = malloc(16);
	scanf("%s", out);
	char input[128] = { 0 };
	DWORD bRet = 0;
	printf("DBG: wysylam...\n");
	if (!(DeviceIoControl(dev, KILL_IOCTL_CODE, out, sizeof(out), input, sizeof(input), &bRet, (LPOVERLAPPED)NULL ))) {
		printf("ERR: wysylanie do drv dostalo wylewu\n");
	}
	printf("odpowiedz od VolDrv:\n\n%s", input);
	CloseHandle(dev);
	getc(stdin);
	return 0;
}