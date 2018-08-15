#include "SocketUtil.h"
#include "Utils.h"

#define DirectoryStr(xdata, out) \
	if(xdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) \
		memcpy(out, "<DIR>", 5)

char* GetDirectoryContent(HANDLE heapHandle, PSTR oneDirectory) {
	if(!SetWorkingDirectory(oneDirectory))
		return NULL;

	const DWORD eachAllocSize = MAX_PATH + 7;
	char *data = HeapAlloc(heapHandle, 
			HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, 
			eachAllocSize);
	//setup
	char ifDirectoryStr[5] = { 0 }; //is "\0\0\0\0\0" if no directory attr, "<DIR>" otherwise
	WIN32_FIND_DATAA findData; //contains directory data

	HANDLE firstFile = FindFirstFile(oneDirectory, &findData);
	if(firstFile == NULL) {
		HeapFree(heapHandle, 0, data);
		return NULL;
	}
	
	DirectoryStr(findData, ifDirectoryStr);
	snprintf(data, eachAllocSize, "%s %s\n", findData.cFileName, ifDirectoryStr);
	while(FindNextFile(firstFile, &findData)) {
		DWORD actualSize = HeapSize(heapHandle, 0x0, data); //??
		ZeroMemory(ifDirectoryStr, 5);

		data = HeapReAlloc(heapHandle, 
				HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, 
				data, 
				actualSize + eachAllocSize);
		
		DirectoryStr(findData, ifDirectoryStr);
		snprintf(data + actualSize, eachAllocSize, "%s %s\n", findData.cFileName, ifDirectoryStr);
	}
	
	FindClose(firstFile);
	
	return data;
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


