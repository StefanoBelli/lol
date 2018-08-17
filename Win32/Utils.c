#include <Windows.h>
#include <tlhelp32.h>
#include <string.h>
#include "SocketUtil.h"
#include "Utils.h"

#define DIR_OR_NOT(xdata) ((xdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "<DIR>" : "<FIL>")

#define FormatFileData(xdata, ydest, zlen) \
	snprintf(ydest, zlen, "%s %s\n", DIR_OR_NOT(xdata), xdata.cFileName)

char* GetDirectoryContent(HANDLE heapHandle, LPCSTR directory) {
	WIN32_FIND_DATAA eachDirectoryData;
	HANDLE fileLookupHandle = FindFirstFileA(directory, &eachDirectoryData);

	if (fileLookupHandle == NULL)
		return NULL; //no dynamic allocation performed

	const DWORD eachAllocRequiredSpace = MAX_PATH + 7;

	char* fullListing = (char*) HeapAlloc(heapHandle,
		HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
		eachAllocRequiredSpace);

	if (fullListing == NULL)
		return NULL; //HeapAlloc failed

	FormatFileData(eachDirectoryData, fullListing, eachAllocRequiredSpace);

	char eachDirectoryFormattedText[MAX_PATH + 7];
	ZeroMemory(eachDirectoryFormattedText, eachAllocRequiredSpace);

	while (FindNextFileA(fileLookupHandle, &eachDirectoryData) 
		&& GetLastError() != ERROR_FILE_NOT_FOUND) {
		fullListing = (char*) HeapReAlloc(heapHandle,
			HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
			fullListing,
			strlen(fullListing) + eachAllocRequiredSpace);

		FormatFileData(eachDirectoryData, eachDirectoryFormattedText, eachAllocRequiredSpace);
		strncat(fullListing, eachDirectoryFormattedText, eachAllocRequiredSpace);
		ZeroMemory(eachDirectoryFormattedText, eachAllocRequiredSpace);
	}

	FindClose(fileLookupHandle);
	return fullListing;
}

SPAWNED_PROCESS_INFO SpawnNewProcess(LPSTR commandLine) {
	SPAWNED_PROCESS_INFO process;

	PROCESS_INFORMATION procInfo;
	STARTUPINFO startupInfo;

	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&process, sizeof(SPAWNED_PROCESS_INFO));

	char system32[MAX_PATH];
	ZeroMemory(system32, MAX_PATH);

	const char* systemDrive = getenv("SystemDrive");
	*system32 = *systemDrive;
	*(system32 + 1) = *(systemDrive + 1);
	memcpy(system32 + 2, "\\Windows\\System32", sizeof("\\Windows\\System32"));

	char fullCommand[BUFSIZE + 11];
	ZeroMemory(fullCommand, BUFSIZE + 11);
	memcpy(fullCommand, "cmd.exe /c ", 11);
	memcpy(fullCommand + 11, commandLine, strlen(commandLine));

	if(!CreateProcessA(NULL, fullCommand,
				NULL, NULL, FALSE, CREATE_NEW_CONSOLE, 
				NULL, system32, &startupInfo, &procInfo))
		return process;

	process.info = procInfo;
	process.stdOut = startupInfo.hStdOutput;
	process.stdIn = startupInfo.hStdInput;
	process.stdErr = startupInfo.hStdError;
	process.isOk = TRUE;

	return process;
}

char* GetSystemProcessSnapshot(HANDLE heapHandle) {
	HANDLE systemSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 entry;

	if(systemSnapshot == NULL)
		return NULL;

	entry.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(systemSnapshot, &entry)) {
		CloseHandle(systemSnapshot);
		return NULL;
	}

	const DWORD eachLineSize = 14 + MAX_PATH;
	char* snapshot = HeapAlloc(heapHandle, HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, eachLineSize);

	snprintf(snapshot, eachLineSize, "[%d] %s\n", 
			entry.th32ProcessID,
			entry.szExeFile);

	char currentLine[MAX_PATH + 14];
	while(Process32Next(systemSnapshot, &entry)) {
		snapshot = HeapReAlloc(heapHandle, HEAP_GENERATE_EXCEPTIONS |
				HEAP_ZERO_MEMORY, snapshot,
				HeapSize(heapHandle, 0x0, snapshot) + eachLineSize);

		ZeroMemory(currentLine, eachLineSize);
		snprintf(currentLine, eachLineSize, "[%d] %s\n", 
				entry.th32ProcessID,
				entry.szExeFile);
		strncat(snapshot, currentLine, strlen(currentLine));
	}

	CloseHandle(systemSnapshot);
	return snapshot;
}
